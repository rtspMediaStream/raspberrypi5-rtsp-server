#include "utils.h"
#include "ClientSession.h"
#include "RequestHandler.h"
#include "UDPHandler.h"
#include "MediaStreamHandler.h"
#include <thread>

ClientSession::ClientSession(const int tcpSocket, const std::string ip) {
    this->id = (int)utils::GetRanNum(16);
    this->version = id;
    this->tcpSocket = tcpSocket;
    this->ip = ip;

    this->rtpPort = -1;
    this->rtcpPort = -1;
}

int ClientSession::GetSessionId() const { return this->id; }

int ClientSession::GetVersion() const { return this->version; }