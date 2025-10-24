/******************************************************************************\
 * Copyright (c) 2025
 *
 * Author(s):
 *  wwek
 *
 * Description:
 *  DRM status broadcast via Unix Domain Socket
 *  Provides real-time DRM receiver status information to external clients
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

#ifndef STATUSBROADCAST_H
#define STATUSBROADCAST_H

#include "../GlobalDefinitions.h"
#include "../Parameter.h"
#include <string>
#include <vector>
#include <thread>
#include <atomic>

// Forward declarations
class CDRMReceiver;

/**
 * @brief Unix Domain Socket server for broadcasting DRM status
 *
 * Broadcasts JSON-formatted DRM receiver status to connected clients
 * every 500ms. Supports multiple concurrent clients.
 */
class CStatusBroadcast
{
public:
    CStatusBroadcast();
    ~CStatusBroadcast();

    /**
     * @brief Start the status broadcast service
     * @param pReceiver Pointer to DRM receiver instance
     * @param strCustomPath Custom socket path (optional, defaults to /tmp/dream_status_{PID}.sock)
     * @return true if started successfully
     */
    bool Start(CDRMReceiver* pReceiver, const std::string& strCustomPath = "");

    /**
     * @brief Stop the status broadcast service
     */
    void Stop();

    /**
     * @brief Check if service is running
     */
    bool IsRunning() const { return bRunning; }

    /**
     * @brief Get the socket path
     */
    std::string GetSocketPath() const { return strSocketPath; }

private:
    /**
     * @brief Main broadcast loop (runs in separate thread)
     */
    void BroadcastLoop();

    /**
     * @brief Accept new client connections (non-blocking)
     */
    void AcceptNewClients();

    /**
     * @brief Broadcast JSON status to all connected clients
     * @param strJSON JSON string to broadcast
     */
    void BroadcastToClients(const std::string& strJSON);

    /**
     * @brief Collect current DRM status as JSON string
     * @return JSON string containing status information
     */
    std::string CollectStatusJSON();

    /**
     * @brief Convert ETypeRxStatus to integer
     * Reference: KiwiSDR ConsoleIO.cpp:395-403
     */
    static int ETypeRxStatus2int(ETypeRxStatus eStatus);

    /**
     * @brief Convert bandwidth index to kHz value
     * @param iBandwidthIndex Bandwidth index (0-5)
     * @return Bandwidth in kHz (4.5, 5, 9, 10, 18, 20)
     */
    static _REAL GetBandwidthKHz(int iBandwidthIndex);

    /**
     * @brief Escape special characters for JSON string
     * @param str Input string
     * @return JSON-escaped string
     */
    static std::string EscapeJSON(const std::string& str);

    /**
     * @brief Create socket path with PID
     * Format: /tmp/dream_status_{PID}.sock
     */
    static std::string CreateSocketPath();

    // Member variables
    CDRMReceiver*           pDRMReceiver;
    std::string             strSocketPath;
    int                     iServerFd;
    std::vector<int>        vecClientFds;
    std::thread             broadcastThread;
    std::atomic<bool>       bRunning;

    // Update interval in milliseconds (same as KiwiSDR GUI_CONTROL_UPDATE_TIME)
    static const int        UPDATE_INTERVAL_MS = 500;
};

#endif // STATUSBROADCAST_H
