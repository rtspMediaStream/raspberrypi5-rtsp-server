#ifndef RTSP_TCPHANDLER_H
#define RTSP_TCPHANDLER_H

#include "Protos.h"

#include <string>
#include <arpa/inet.h>
#include <unordered_map>
#include <vector>

class TCPHandler {
public:
    TCPHandler(const TCPHandler&) = delete;
    TCPHandler& operator=(const TCPHandler&) = delete;

    static TCPHandler& GetInstance() {
        static TCPHandler instance;
        return instance;
    };

    void CreateTCPSocket();

    int AcceptClientConnection(std::string &_clientIp);

    void CloseClientConnection();

    std::string ReceiveRTSPRequest(int sessionId);

    void SendRTSPResponse(int clientSocket, std::string& response);

    int& GetTCPSocket();

    sockaddr_in& GetTCPAddr();


private:
    TCPHandler();
    ~TCPHandler();

    int tcpPort;
    int tcpSocket;

    sockaddr_in tcpAddr;

    std::unordered_map<int, int> socketTable;
};

#endif //RTSP_TCPHANDLER_H