#ifndef RTSP_CLIENTSESSION_H
#define RTSP_CLIENTSESSION_H

#include "TCPHandler.h"

#include <map>
#include <queue>
#include <mutex>
#include <chrono>
#include <string>
#include <iostream>

class ClientSession {
public:
    ClientSession(int sessionId);

    int GetSessionId() const;

    int GetVersion() const;

    std::string GetState() const;

    std::pair<int, int> GetPort() const;

    void SetPort(int port1, int port2);

    void SetState(const std::string& newState);

private:
    int sessionId, version;
    int rtpPort, rtcpPort;
    std::string state;
    std::mutex mtx;
};

#endif //RTSP_CLIENTSESSION_H