#ifndef RTSP_TCPHANDLER_H
#define RTSP_TCPHANDLER_H

#include "Protos.h"
#include <string>
#include <arpa/inet.h>

class TCPHandler {
public:
    TCPHandler();
    ~TCPHandler();

    static TCPHandler* getInstance() {
        if (instance == NULL)
            instance = new TCPHandler();
        return instance;
    }

    void createTCPSocket();

    int acceptClientConnection();

    std::string receiveRTSPRequest(int clientSocket);

    void sendRTSPResponse(int clientSocket, std::string& response);

    int& getTCPSocket();

    sockaddr_in& getTCPAddr();


private:
    static TCPHandler* instance;

    int tcpPort;
    int tcpSocket;

    sockaddr_in tcpAddr;
};

#endif //RTSP_TCPHANDLER_H