#ifndef RTSP_CLIENTSESSION_H
#define RTSP_CLIENTSESSION_H

#include "RequestHandler.h"
#include "UDPHandler.h"

#include <map>
#include <queue>
#include <mutex>
#include <chrono>
#include <string>
#include <iostream>

struct Info {
    int id;
    int version;
    int tcpSocket;
    int rtpPort;
    int rtcpPort;
    std::string ip;
    std::string state;
};

class ClientSession {
public:
    ClientSession(const std::pair<int, std::string>& newClient);

    int GetSessionId() const;

    int GetVersion() const;

    std::string GetState() const;

    std::pair<int, int> GetPort() const;

    void SetState(const std::string& newState);

private:
    
    std::shared_ptr<Info> info;

    // std::mutex mtx;

    struct Handlers {
        static RequestHandler* requestHandler;
        static UDPHandler* udpHandler;
    };
};

#endif //RTSP_CLIENTSESSION_H