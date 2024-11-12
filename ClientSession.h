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

    int getSessionId() const;

    int getVersion() const;

    std::string getState() const;

    std::pair<int, int> getPort() const;

    void setPort(int port1, int port2);

    void setState(const std::string& newState);

private:
    int sessionId, version;
    int rtpPort, rtcpPort;
    std::string state;
    std::mutex mtx;
};

#endif //RTSP_CLIENTSESSION_H