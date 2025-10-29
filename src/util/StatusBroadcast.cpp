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
#include "../tables/TableFAC.h"
#include "../datadecoding/DataDecoder.h"

#ifndef _WIN32
// Unix-specific headers for Unix Domain Sockets
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <cerrno>
#endif

#include <cstring>
#include <map>
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
#ifdef _WIN32
    // Windows: Unix Domain Sockets not supported
    // TODO: Implement Windows Named Pipes or TCP socket alternative
    (void)pReceiver;
    (void)strCustomPath;
    fprintf(stderr, "StatusBroadcast: Not implemented on Windows\n");
    return false;
#else
    // Unix/Linux/macOS: Use Unix Domain Sockets
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

    if (::bind(iServerFd, (struct sockaddr*)&addr, sizeof(addr)) < 0)
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
#endif
}

void CStatusBroadcast::Stop()
{
#ifdef _WIN32
    // Windows: Nothing to stop (not implemented)
    return;
#else
    // Unix/Linux/macOS
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
#endif
}

void CStatusBroadcast::BroadcastLoop()
{
#ifdef _WIN32
    // Windows: Not implemented
    return;
#else
    // Unix/Linux/macOS
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
#endif
}

void CStatusBroadcast::AcceptNewClients()
{
#ifndef _WIN32
    // Unix/Linux/macOS only
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
#endif
}

void CStatusBroadcast::BroadcastToClients(const std::string& strJSON)
{
#ifndef _WIN32
    // Unix/Linux/macOS only
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
#endif
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

    // DRM time information (from DRM transmission)
    json << "\"drm_time\":{";

    if (Parameters.iYear == 0 && Parameters.iMonth == 0 && Parameters.iDay == 0 &&
        Parameters.iUTCHour == 0 && Parameters.iUTCMin == 0)
    {
        // No DRM time available
        json << "\"valid\":false";
    }
    else
    {
        json << "\"valid\":true,";

        // Raw DRM time fields (UTC time from DRM transmission)
        json << "\"year\":" << Parameters.iYear << ",";
        json << "\"month\":" << Parameters.iMonth << ",";
        json << "\"day\":" << Parameters.iDay << ",";
        json << "\"hour\":" << Parameters.iUTCHour << ",";
        json << "\"min\":" << Parameters.iUTCMin << ",";

        // UTC timestamp
        struct tm timeinfo = {0};
        timeinfo.tm_year = Parameters.iYear - 1900;
        timeinfo.tm_mon = Parameters.iMonth - 1;
        timeinfo.tm_mday = Parameters.iDay;
        timeinfo.tm_hour = Parameters.iUTCHour;
        timeinfo.tm_min = Parameters.iUTCMin;
        timeinfo.tm_sec = 0;

#ifdef _WIN32
        time_t drm_timestamp = _mkgmtime(&timeinfo);
#else
        time_t drm_timestamp = timegm(&timeinfo);
#endif

        if (drm_timestamp != -1)
        {
            json << "\"timestamp\":" << drm_timestamp << ",";
        }
        else
        {
            json << "\"timestamp\":0,";
        }

        // Local time offset information (if available)
        json << "\"has_local_offset\":" << (Parameters.bValidUTCOffsetAndSense ? "true" : "false");

        if (Parameters.bValidUTCOffsetAndSense)
        {
            // UTC offset in half-hour units (±0.5h resolution)
            int offset_minutes = Parameters.iUTCOff * 30;
            if (Parameters.iUTCSense == 1)
            {
                offset_minutes = -offset_minutes;  // Negative offset
            }

            json << ",\"offset_min\":" << offset_minutes;
        }
    }

    json << "},";

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

    // Extended signal quality parameters (only with signal)
    if (signal)
    {
        // WMER (Weighted Modulation Error Ratio) - MSC quality indicator
        if (Parameters.rWMERMSC >= 0.0)
        {
            json << ",\"wmer_db\":" << Parameters.rWMERMSC;
        }

        // MER (Modulation Error Ratio) - MSC quality indicator
        if (Parameters.rMER >= 0.0)
        {
            json << ",\"mer_db\":" << Parameters.rMER;
        }

        // Doppler frequency estimate (Hz) - channel variation speed
        if (Parameters.rSigmaEstimate >= 0.0)
        {
            json << ",\"doppler_hz\":" << std::setprecision(2) << Parameters.rSigmaEstimate;
            json << std::setprecision(1);
        }

        // Delay spread (ms) - multipath propagation
        if (Parameters.rMinDelay >= 0.0)
        {
            json << ",\"delay_min_ms\":" << std::setprecision(2) << Parameters.rMinDelay;

            if (Parameters.rMaxDelay >= 0.0)
            {
                json << ",\"delay_max_ms\":" << std::setprecision(2) << Parameters.rMaxDelay;
            }
            json << std::setprecision(1);
        }
    }

    json << "},";

    // Frequency parameters
    json << "\"frequency\":{";
    json << "\"dc_offset_hz\":" << std::setprecision(2) << Parameters.GetDCFrequency();

    _REAL rSampleOffset = Parameters.rResampleOffset;
    _REAL rSampleRate = Parameters.GetSigSampleRate();
    int iSampleOffsetPPM = (rSampleRate > 0) ? (int)(rSampleOffset / rSampleRate * 1e6) : 0;

    json << ",\"sample_offset_hz\":" << std::setprecision(2) << rSampleOffset;
    json << ",\"sample_offset_ppm\":" << iSampleOffsetPPM;
    json << std::setprecision(1);
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

                    // Audio mode (Mono/Stereo/P-Stereo)
                    json << ",\"audio_mode\":";
                    switch (service.AudioParam.eAudioMode)
                    {
                    case CAudioParam::AM_MONO:
                        json << "\"Mono\"";
                        break;
                    case CAudioParam::AM_STEREO:
                        json << "\"Stereo\"";
                        break;
                    case CAudioParam::AM_P_STEREO:
                        json << "\"P-Stereo\"";
                        break;
                    default:
                        json << "\"Unknown\"";
                        break;
                    }

                    // Protection mode (EEP/UEP)
                    _REAL rPartABLenRat = Parameters.PartABLenRatio(i);
                    if (rPartABLenRat != (_REAL) 0.0)
                    {
                        // UEP mode with percentage
                        json << ",\"protection_mode\":\"UEP\"";
                        json << ",\"protection_percent\":" << std::setprecision(1) << (rPartABLenRat * 100.0);
                    }
                    else
                    {
                        // EEP mode
                        json << ",\"protection_mode\":\"EEP\"";
                    }

                    // Text message if available
                    if (service.AudioParam.bTextflag && !service.AudioParam.strTextMessage.empty())
                    {
                        json << ",\"text\":\"" << EscapeJSON(service.AudioParam.strTextMessage) << "\"";
                    }

                    // Language information (audio services only)
                    const std::string& strLangCode = service.strLanguageCode;
                    if (!strLangCode.empty() && strLangCode != "---")
                    {
                        // SDC language (ISO 639-2, 3-letter code) - preferred
                        json << ",\"language\":{";
                        json << "\"code\":\"" << strLangCode << "\",";
                        json << "\"name\":\"" << EscapeJSON(GetISOLanguageName(strLangCode)) << "\"";
                        json << "}";
                    }
                    else if (service.iLanguage > 0 && service.iLanguage < LEN_TABLE_LANGUAGE_CODE)
                    {
                        // FAC language (0-15) - fallback
                        json << ",\"language\":{";
                        json << "\"fac_id\":" << service.iLanguage << ",";
                        json << "\"name\":\"" << EscapeJSON(strTableLanguageCode[service.iLanguage]) << "\"";
                        json << "}";
                    }

                    // Program type (audio services only)
                    if (service.iServiceDescr > 0 && service.iServiceDescr < LEN_TABLE_PROG_TYPE_CODE)
                    {
                        json << ",\"program_type\":{";
                        json << "\"id\":" << service.iServiceDescr << ",";
                        json << "\"name\":\"" << EscapeJSON(strTableProgTypCod[service.iServiceDescr]) << "\"";
                        json << "}";
                    }
                }
                else
                {
                    json << std::setprecision(2);
                    json << "\"bitrate_kbps\":" << Parameters.GetBitRateKbps(i, true);
                    json << std::setprecision(1);
                }

                // Country code (both audio and data services)
                const std::string& strCntryCode = service.strCountryCode;
                if (!strCntryCode.empty() && strCntryCode != "--")
                {
                    json << ",\"country\":{";
                    json << "\"code\":\"" << strCntryCode << "\",";
                    json << "\"name\":\"" << EscapeJSON(GetISOCountryName(strCntryCode)) << "\"";
                    json << "}";
                }

                json << "}";
            }
        }
        json << "]";
    }

    // Media availability detection (Program Guide, Journaline, Slideshow)
    json << ",\"media\":{";

    // Get DataDecoder instance
    CDataDecoder* pDataDecoder = pDRMReceiver->GetDataDecoder();

    bool bHasProgramGuide = false;
    bool bHasJournaline = false;
    bool bHasSlideshow = false;
    bool bHasMediaContent = false;
    std::ostringstream mediaContentJson;

    if (pDataDecoder != nullptr)
    {
        // Check all packet IDs for available media types
        for (int iPacketID = 0; iPacketID < MAX_NUM_PACK_PER_STREAM; iPacketID++)
        {
            CMOTDABDec* pMOTApp = pDataDecoder->getApplication(iPacketID);
            if (pMOTApp != nullptr && pMOTApp->NewObjectAvailable())
            {
                // Get application type for this specific packet ID
                CDataDecoder::EAppType eAppType = pDataDecoder->GetAppType(iPacketID);

                // Extract new object from queue (EVENT-DRIVEN: only when available)
                CMOTObject NewObj;
                pMOTApp->GetNextObject(NewObj);

                // Generate unique key for tracking: "type_transportID"
                std::ostringstream keyStream;
                keyStream << (int)eAppType << "_" << NewObj.TransportID;
                std::string strMediaKey = keyStream.str();

                // Check if this is truly new content (compare with last pushed timestamp)
                time_t currentObjTime = time(nullptr);  // Use current time as timestamp
                bool bIsNewContent = false;

                auto it = mapLastPushedMedia.find(strMediaKey);
                if (it == mapLastPushedMedia.end() || NewObj.iUniqueBodyVersion != it->second)
                {
                    bIsNewContent = true;
                    mapLastPushedMedia[strMediaKey] = NewObj.iUniqueBodyVersion;  // Store version instead of timestamp
                }

                // Set availability flags
                switch (eAppType)
                {
                    case CDataDecoder::AT_EPG:
                        bHasProgramGuide = true;
                        // Extract EPG content if new
                        if (bIsNewContent && NewObj.Body.vecData.Size() > 0)
                        {
                            if (bHasMediaContent) mediaContentJson << ",";
                            mediaContentJson << "\"program_guide\":{";
                            mediaContentJson << "\"timestamp\":" << currentObjTime;
                            mediaContentJson << ",\"name\":\"" << JsonEscape(NewObj.strName) << "\"";
                            mediaContentJson << ",\"description\":\"" << JsonEscape(NewObj.strContentDescription) << "\"";
                            mediaContentJson << ",\"size\":" << NewObj.Body.vecData.Size();
                            // Base64 encode body data
                            std::string base64Data = Base64Encode(
                                reinterpret_cast<const unsigned char*>(&NewObj.Body.vecData[0]),
                                NewObj.Body.vecData.Size()
                            );
                            mediaContentJson << ",\"data\":\"" << base64Data << "\"";
                            mediaContentJson << "}";
                            bHasMediaContent = true;
                        }
                        break;

                    case CDataDecoder::AT_JOURNALINE:
                        bHasJournaline = true;
                        // Journaline uses different handling via GetNews()
                        // For now, just mark as available
                        if (bIsNewContent)
                        {
                            if (bHasMediaContent) mediaContentJson << ",";
                            mediaContentJson << "\"journaline\":{";
                            mediaContentJson << "\"timestamp\":" << currentObjTime;
                            mediaContentJson << ",\"available\":true";
                            mediaContentJson << "}";
                            bHasMediaContent = true;
                        }
                        break;

                    case CDataDecoder::AT_MOTSLIDESHOW:
                        bHasSlideshow = true;
                        // Extract Slideshow image if new
                        if (bIsNewContent && NewObj.Body.vecData.Size() > 0)
                        {
                            if (bHasMediaContent) mediaContentJson << ",";
                            mediaContentJson << "\"slideshow\":{";
                            mediaContentJson << "\"timestamp\":" << currentObjTime;
                            mediaContentJson << ",\"name\":\"" << JsonEscape(NewObj.strName) << "\"";
                            mediaContentJson << ",\"mime\":\"" << JsonEscape(NewObj.strMimeType) << "\"";
                            mediaContentJson << ",\"size\":" << NewObj.Body.vecData.Size();
                            // Base64 encode image data
                            std::string base64Data = Base64Encode(
                                reinterpret_cast<const unsigned char*>(&NewObj.Body.vecData[0]),
                                NewObj.Body.vecData.Size()
                            );
                            mediaContentJson << ",\"data\":\"" << base64Data << "\"";
                            mediaContentJson << "}";
                            bHasMediaContent = true;
                        }
                        break;

                    default:
                        break;
                }
            }
        }
    }

    json << "\"program_guide\":" << (bHasProgramGuide ? "true" : "false");
    json << ",\"journaline\":" << (bHasJournaline ? "true" : "false");
    json << ",\"slideshow\":" << (bHasSlideshow ? "true" : "false");
    json << "}";

    // Append media_content section only if there's new content (ONE-TIME PUSH)
    if (bHasMediaContent)
    {
        json << ",\"media_content\":{" << mediaContentJson.str() << "}";
    }

    json << "}";

    return json.str();
}

std::string CStatusBroadcast::Base64Encode(const unsigned char* data, size_t len)
{
    static const char base64_chars[] =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

    std::string ret;
    int i = 0;
    int j = 0;
    unsigned char char_array_3[3];
    unsigned char char_array_4[4];

    while (len--) {
        char_array_3[i++] = *(data++);
        if (i == 3) {
            char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
            char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
            char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
            char_array_4[3] = char_array_3[2] & 0x3f;

            for(i = 0; (i <4) ; i++)
                ret += base64_chars[char_array_4[i]];
            i = 0;
        }
    }

    if (i)
    {
        for(j = i; j < 3; j++)
            char_array_3[j] = '\0';

        char_array_4[0] = ( char_array_3[0] & 0xfc) >> 2;
        char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
        char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);

        for (j = 0; (j < i + 1); j++)
            ret += base64_chars[char_array_4[j]];

        while((i++ < 3))
            ret += '=';
    }

    return ret;
}

std::string CStatusBroadcast::JsonEscape(const std::string& str)
{
    std::ostringstream o;
    for (auto c : str) {
        switch (c) {
            case '"':  o << "\\\""; break;
            case '\\': o << "\\\\"; break;
            case '\b': o << "\\b"; break;
            case '\f': o << "\\f"; break;
            case '\n': o << "\\n"; break;
            case '\r': o << "\\r"; break;
            case '\t': o << "\\t"; break;
            default:
                if ('\x00' <= c && c <= '\x1f') {
                    o << "\\u" << std::hex << std::setw(4) << std::setfill('0') << (int)c;
                } else {
                    o << c;
                }
        }
    }
    return o.str();
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
    // Preserve UTF-8 multi-byte sequences (for non-ASCII labels)
    std::ostringstream escaped;
    for (unsigned char c : str)
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
            // Only escape control characters (0x00-0x1F)
            // Preserve UTF-8 multi-byte sequences (0x80-0xFF)
            if (c < 0x20)
            {
                // Escape control characters
                escaped << "\\u" << std::hex << std::setw(4) << std::setfill('0') << (int)c;
            }
            else
            {
                // Preserve all other bytes (including UTF-8 sequences)
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
