#include "utils.h"
#include "ClientSession.h"
#include "TCPHandler.h"
#include "UDPHandler.h"
#include "RequestHandler.h"
#include "MediaStreamHandler.h"

#include <string>
#include <thread>
#include <iostream>

#define TCP TCPHandler::GetInstance()

TCPHandler* TCPHandler::instance = nullptr;

int main() {
    TCP.CreateTCPSocket();

    std::cout << "Start RTSP server" << std::endl;

    while (true) {
        std::pair<int, std::string> newClient = TCP.AcceptClientConnection();

        std::cout << "Client connected" << std::endl;

        ClientSession* clientSession = new ClientSession(newClient);
    }

    return 0;
}
