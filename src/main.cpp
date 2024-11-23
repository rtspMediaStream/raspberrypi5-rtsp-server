#include "utils.h"
#include "ClientSession.h"
#include "TCPHandler.h"
#include "UDPHandler.h"
#include "RequestHandler.h"
#include "MediaStreamHandler.h"

#include <string>
#include <thread>
#include <iostream>


int main() {
    TCPHandler::GetInstance().CreateTCPSocket();

    std::cout << "Start RTSP server" << std::endl;

    while (true) {
        std::pair<int, std::string> newClient = TCPHandler::GetInstance().AcceptClientConnection();

        std::cout << "Client connected" << std::endl;

        ClientSession* clientSession = new ClientSession(newClient);
        std::cout << "Main while loop." << std::endl;
    }

    return 0;
}
