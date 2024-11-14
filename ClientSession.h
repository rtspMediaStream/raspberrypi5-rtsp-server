#ifndef RTSP_CLIENTSESSION_H
#define RTSP_CLIENTSESSION_H

#include "RequestHandler.h"

#include <map>
#include <queue>
#include <mutex>
#include <chrono>
#include <string>
#include <iostream>

class ClientSession {
public:
    ClientSession();

    int GetSessionId() const;

    int GetVersion() const;

    std::string GetState() const;

    std::pair<int, int> GetPort() const;

    // void SetPort(int port1, int port2);

    void SetState(const std::string& newState);

private:
    int sessionId;
    int version;
    int socket;
    int rtpPort;
    int rtcpPort;

    std::string state;
    // std::mutex mtx;

    struct Handlers {
        static RequestHandler* requestHandler;

    };
};

#endif //RTSP_CLIENTSESSION_H