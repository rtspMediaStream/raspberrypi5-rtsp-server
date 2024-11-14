#include "ClientSession.h"
#include <mutex>
#include <string>

ClientSession::ClientSession(int sessionId)
        : sessionId(sessionId), version(sessionId), state("INIT") {}

int ClientSession::GetSessionId() const { return sessionId; }

int ClientSession::GetVersion() const { return version; }

std::string ClientSession::GetState() const { return state; }

std::pair<int, int> ClientSession::GetPort() const { return {rtpPort, rtcpPort}; }

void ClientSession::SetPort(int port1, int port2) {
    rtpPort = port1;
    rtcpPort = port2;

    SOCK.createUDPSocket(rtpPort, rtcpPort);
}

void ClientSession::SetState(const std::string& newState) {
    std::lock_guard<std::mutex> lock(mtx);
    state = newState;
    version++;
}