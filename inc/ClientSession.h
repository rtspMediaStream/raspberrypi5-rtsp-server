#ifndef RTSP_CLIENTSESSION_H
#define RTSP_CLIENTSESSION_H

#include <map>
#include <queue>
#include <mutex>
#include <chrono>
#include <string>
#include <iostream>
#include <memory>

class RequestHandler;
class UDPHandler;
class MediaStreamHandler;

class ClientSession {
public:
    ClientSession(const int tcpSocket, const std::string ip);
    
    int GetSessionId() const;

    int GetVersion() const;

    inline int GetID() { return this->tcpSocket; };
    inline int GetTCPSocket() { return this->tcpSocket; };
    inline int GetRTPPort() { return this->rtpPort; };
    inline int GetRTCPPort() { return this->rtcpPort; };
    inline std::string GetIP() { return this->ip; };

    inline void SetRTPPort(int rtpPort) { this->rtpPort = rtpPort; };
    inline void SetRTCPPort(int rtcpPort) { this->rtcpPort = rtcpPort; };
    ///RTP 만들기

private:
    int id;
    int version;
    int tcpSocket;
    int rtpPort;
    int rtcpPort;
    std::string ip;
    
    RequestHandler* requestHandler;
};

#endif //RTSP_CLIENTSESSION_H