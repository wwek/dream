/******************************************************************************\
 * Copyright (c) 2025
 *
 * Author(s):
 *  wwek
 *
 * Description:
 *  Implementation of DRM status broadcast via Unix Domain Socket
 *  Reference: KiwiSDR extensions/DRM/dream/linux/ConsoleIO.cpp
 *
 ******************************************************************************
 *
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation; either version 2 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc.,
 * 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
\******************************************************************************/

#include "StatusBroadcast.h"
#include "../DrmReceiver.h"
#include "../Parameter.h"
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <cerrno>
#include <cstring>
#include <cstdio>
#include <ctime>
#include <sstream>
#include <iomanip>
#include <chrono>

CStatusBroadcast::CStatusBroadcast()
    : pDRMReceiver(nullptr)
    , strSocketPath("")
    , iServerFd(-1)
    , bRunning(false)
{
}

CStatusBroadcast::~CStatusBroadcast()
{
    Stop();
}

bool CStatusBroadcast::Start(CDRMReceiver* pReceiver, const std::string& strCustomPath)
{
    if (bRunning)
        return false;

    if (pReceiver == nullptr)
        return false;

    pDRMReceiver = pReceiver;

    // Use custom path if provided, otherwise generate default path
    if (!strCustomPath.empty())
        strSocketPath = strCustomPath;
    else
        strSocketPath = CreateSocketPath();

    // Remove old socket file if exists (previous unclean exit)
    struct stat st;
    if (stat(strSocketPath.c_str(), &st) == 0)
    {
        fprintf(stderr, "StatusBroadcast: Removing stale socket file %s\n", strSocketPath.c_str());
        unlink(strSocketPath.c_str());
    }

    // Create Unix Domain Socket
    iServerFd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (iServerFd < 0)
    {
        fprintf(stderr, "StatusBroadcast: Failed to create socket: %s\n", strerror(errno));
        return false;
    }

    // Set non-blocking mode
    int flags = fcntl(iServerFd, F_GETFL, 0);
    fcntl(iServerFd, F_SETFL, flags | O_NONBLOCK);

    // Bind socket
    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, strSocketPath.c_str(), sizeof(addr.sun_path) - 1);

    if (bind(iServerFd, (struct sockaddr*)&addr, sizeof(addr)) < 0)
    {
        fprintf(stderr, "StatusBroadcast: Failed to bind socket to %s: %s\n",
                strSocketPath.c_str(), strerror(errno));
        close(iServerFd);
        iServerFd = -1;
        return false;
    }

    // Listen for connections
    if (listen(iServerFd, 5) < 0)
    {
        fprintf(stderr, "StatusBroadcast: Failed to listen on socket: %s\n", strerror(errno));
        close(iServerFd);
        iServerFd = -1;
        unlink(strSocketPath.c_str());
        return false;
    }

    // Start broadcast thread
    bRunning = true;
    broadcastThread = std::thread(&CStatusBroadcast::BroadcastLoop, this);

    fprintf(stderr, "StatusBroadcast: Started on %s\n", strSocketPath.c_str());
    return true;
}

void CStatusBroadcast::Stop()
{
    if (!bRunning)
        return;

    bRunning = false;

    // Wait for thread to finish
    if (broadcastThread.joinable())
        broadcastThread.join();

    // Close all client connections
    for (int fd : vecClientFds)
        close(fd);
    vecClientFds.clear();

    // Close server socket
    if (iServerFd >= 0)
    {
        close(iServerFd);
        iServerFd = -1;
    }

    // Remove socket file
    if (!strSocketPath.empty())
    {
        if (unlink(strSocketPath.c_str()) == 0)
        {
            fprintf(stderr, "StatusBroadcast: Removed socket file %s\n", strSocketPath.c_str());
        }
        else
        {
            fprintf(stderr, "StatusBroadcast: Failed to remove socket file %s: %s\n",
                    strSocketPath.c_str(), strerror(errno));
        }
    }

    fprintf(stderr, "StatusBroadcast: Stopped\n");
}

void CStatusBroadcast::BroadcastLoop()
{
    while (bRunning)
    {
        // Accept new client connections
        AcceptNewClients();

        // Collect status and broadcast
        std::string strJSON = CollectStatusJSON();
        BroadcastToClients(strJSON);

        // Sleep for update interval
        std::this_thread::sleep_for(std::chrono::milliseconds(UPDATE_INTERVAL_MS));
    }
}

void CStatusBroadcast::AcceptNewClients()
{
    // Non-blocking accept
    int client_fd = accept(iServerFd, nullptr, nullptr);
    if (client_fd >= 0)
    {
        // Set non-blocking mode for client socket
        int flags = fcntl(client_fd, F_GETFL, 0);
        fcntl(client_fd, F_SETFL, flags | O_NONBLOCK);

        vecClientFds.push_back(client_fd);
        fprintf(stderr, "StatusBroadcast: Client connected (fd=%d, total=%zu)\n",
                client_fd, vecClientFds.size());
    }
}

void CStatusBroadcast::BroadcastToClients(const std::string& strJSON)
{
    if (vecClientFds.empty())
        return;

    // Add newline for line-based parsing
    std::string message = strJSON + "\n";
    const char* data = message.c_str();
    size_t len = message.size();

    // Broadcast to all clients
    auto it = vecClientFds.begin();
    while (it != vecClientFds.end())
    {
        ssize_t sent = send(*it, data, len, MSG_NOSIGNAL);
        if (sent < 0)
        {
            // Client disconnected or error
            fprintf(stderr, "StatusBroadcast: Client disconnected (fd=%d)\n", *it);
            close(*it);
            it = vecClientFds.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

std::string CStatusBroadcast::CollectStatusJSON()
{
    if (pDRMReceiver == nullptr)
        return "{}";

    CParameter& Parameters = *pDRMReceiver->GetParameters();

    // Quick snapshot of all status values
    // Reference: KiwiSDR ConsoleIO.cpp:156-296

    int msc = ETypeRxStatus2int(Parameters.ReceiveStatus.SLAudio.GetStatus());
    int sdc = ETypeRxStatus2int(Parameters.ReceiveStatus.SDC.GetStatus());
    int fac = ETypeRxStatus2int(Parameters.ReceiveStatus.FAC.GetStatus());
    int timeSync = ETypeRxStatus2int(Parameters.ReceiveStatus.TSync.GetStatus());
    int frame = ETypeRxStatus2int(Parameters.ReceiveStatus.FSync.GetStatus());

    ETypeRxStatus soundCardStatusI = Parameters.ReceiveStatus.InterfaceI.GetStatus();
    ETypeRxStatus soundCardStatusO = Parameters.ReceiveStatus.InterfaceO.GetStatus();
    int inter = ETypeRxStatus2int(soundCardStatusO == NOT_PRESENT ||
        (soundCardStatusI != NOT_PRESENT && soundCardStatusI != RX_OK) ? soundCardStatusI : soundCardStatusO);

    _REAL rIFLevel = Parameters.GetIFSignalLevel();
    _REAL rSNR = 0.0;
    int iRobustness = -1;
    int iBandwidth = -1;

    int signal = (pDRMReceiver->GetAcquiState() == AS_WITH_SIGNAL);

    // Extended parameters (only available with signal)
    int iInterleaver = -1;
    int iSDCMode = -1;
    int iMSCMode = -1;
    int iProtLevelA = -1;
    int iProtLevelB = -1;
    int iNumAudioServices = 0;
    int iNumDataServices = 0;
    _REAL rBandwidthKHz = 0.0;

    if (signal)
    {
        rSNR = Parameters.GetSNR();
        iRobustness = (int)Parameters.GetWaveMode();
        iBandwidth = (int)Parameters.GetSpectrumOccup();

        // Get extended parameters
        iInterleaver = (int)Parameters.eSymbolInterlMode;
        iSDCMode = (int)Parameters.eSDCCodingScheme;
        iMSCMode = (int)Parameters.eMSCCodingScheme;
        iProtLevelA = Parameters.MSCPrLe.iPartA;
        iProtLevelB = Parameters.MSCPrLe.iPartB;
        iNumAudioServices = Parameters.iNumAudioService;
        iNumDataServices = Parameters.iNumDataService;

        // Convert bandwidth index to kHz
        rBandwidthKHz = GetBandwidthKHz(iBandwidth);
    }

    // Build JSON string (Phase 2 - Complete version)
    std::ostringstream json;
    json << std::fixed << std::setprecision(1);
    json << "{";
    json << "\"timestamp\":" << std::time(nullptr) << ",";

    // Status indicators
    json << "\"status\":{";
    json << "\"io\":" << inter << ",";
    json << "\"time\":" << timeSync << ",";
    json << "\"frame\":" << frame << ",";
    json << "\"fac\":" << fac << ",";
    json << "\"sdc\":" << sdc << ",";
    json << "\"msc\":" << msc;
    json << "},";

    // Signal quality
    json << "\"signal\":{";
    json << "\"if_level_db\":" << rIFLevel << ",";
    json << "\"snr_db\":" << rSNR;
    json << "}";

    if (signal)
    {
        // DRM mode parameters
        json << ",\"mode\":{";
        json << "\"robustness\":" << iRobustness << ",";
        json << "\"bandwidth\":" << iBandwidth << ",";
        json << "\"bandwidth_khz\":" << std::setprecision(1) << rBandwidthKHz << ",";
        json << "\"interleaver\":" << iInterleaver;
        json << "},";

        // Coding and protection
        json << "\"coding\":{";
        json << "\"sdc_qam\":" << iSDCMode << ",";
        json << "\"msc_qam\":" << iMSCMode << ",";
        json << "\"protection_a\":" << iProtLevelA << ",";
        json << "\"protection_b\":" << iProtLevelB;
        json << "},";

        // Service counts
        json << "\"services\":{";
        json << "\"audio\":" << iNumAudioServices << ",";
        json << "\"data\":" << iNumDataServices;
        json << "}";

        // Service details array
        json << ",\"service_list\":[";
        bool bFirstService = true;
        for (int i = 0; i < MAX_NUM_SERVICES; i++)
        {
            CService service = Parameters.Service[i];
            if (service.IsActive())
            {
                if (!bFirstService) json << ",";
                bFirstService = false;

                json << "{";
                json << "\"id\":\"" << std::hex << std::uppercase << service.iServiceID << std::dec << "\",";
                json << "\"label\":\"" << EscapeJSON(service.strLabel) << "\",";
                json << "\"is_audio\":" << (service.eAudDataFlag == CService::SF_AUDIO ? "true" : "false") << ",";

                if (service.eAudDataFlag == CService::SF_AUDIO)
                {
                    json << "\"audio_coding\":" << (int)service.AudioParam.eAudioCoding << ",";
                    json << std::setprecision(2);
                    json << "\"bitrate_kbps\":" << Parameters.GetBitRateKbps(i, false);
                    json << std::setprecision(1);

                    // Text message if available
                    if (service.AudioParam.bTextflag && !service.AudioParam.strTextMessage.empty())
                    {
                        json << ",\"text\":\"" << EscapeJSON(service.AudioParam.strTextMessage) << "\"";
                    }
                }
                else
                {
                    json << std::setprecision(2);
                    json << "\"bitrate_kbps\":" << Parameters.GetBitRateKbps(i, true);
                    json << std::setprecision(1);
                }

                json << "}";
            }
        }
        json << "]";
    }

    json << "}";

    return json.str();
}

int CStatusBroadcast::ETypeRxStatus2int(ETypeRxStatus eStatus)
{
    // Reference: KiwiSDR ConsoleIO.cpp:395-403
    switch (eStatus)
    {
    case RX_OK:      return 0;
    case CRC_ERROR:  return 1;
    case DATA_ERROR: return 2;
    case NOT_PRESENT: return -1;
    default:         return -1;
    }
}

_REAL CStatusBroadcast::GetBandwidthKHz(int iBandwidthIndex)
{
    // Convert bandwidth index to kHz value
    // Reference: DRM standard bandwidth values
    switch (iBandwidthIndex)
    {
    case 0: return 4.5;   // SO_0: 4.5 kHz
    case 1: return 5.0;   // SO_1: 5 kHz
    case 2: return 9.0;   // SO_2: 9 kHz
    case 3: return 10.0;  // SO_3: 10 kHz
    case 4: return 18.0;  // SO_4: 18 kHz
    case 5: return 20.0;  // SO_5: 20 kHz
    default: return 0.0;
    }
}

std::string CStatusBroadcast::EscapeJSON(const std::string& str)
{
    // Escape special characters for JSON string
    std::ostringstream escaped;
    for (char c : str)
    {
        switch (c)
        {
        case '"':  escaped << "\\\""; break;
        case '\\': escaped << "\\\\"; break;
        case '\b': escaped << "\\b";  break;
        case '\f': escaped << "\\f";  break;
        case '\n': escaped << "\\n";  break;
        case '\r': escaped << "\\r";  break;
        case '\t': escaped << "\\t";  break;
        default:
            if (c < 32 || c > 126)
            {
                // Escape non-printable characters
                escaped << "\\u" << std::hex << std::setw(4) << std::setfill('0') << (int)(unsigned char)c;
            }
            else
            {
                escaped << c;
            }
            break;
        }
    }
    return escaped.str();
}

std::string CStatusBroadcast::CreateSocketPath()
{
    return std::string("/tmp/dream_status.sock");
}
