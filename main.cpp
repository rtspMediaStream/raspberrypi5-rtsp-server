#include "utils.h"
#include "ClientSession.h"
#include "TCPHandler.h"
#include "UDPHandler.h"
#include "RequestHandler.h"
#include "MediaStreamHandler.h"

#include <thread>
#include <iostream>

#define TCP TCPHandler::getInstance()

TCPHandler* TCPHandler::instance = nullptr;

int main() {
    // TCP 소켓 생성
    TCP->createTCPSocket();

    std::cout << "Start RTSP server" << std::endl;

    while (true) {
        // wait for client
        int clientSocket = TCP->acceptClientConnection();
        if (clientSocket == -1) {
            std::cerr << "Error: Client accept failed" << std::endl;
            continue; // 오류 시 다음 클라이언트 접속 대기
        }

        std::cout << "Client connected" << std::endl;

        auto mediaStreamHandler = MediaStreamHandler();
        auto requestHandler = RequestHandler(mediaStreamHandler);
        auto clientSession = ClientSession((int)utils::getRanNum(16));

        // 클라이언트 세션을 새로운 스레드에서 처리
        std::thread clientThread(&RequestHandler::handleRequest, requestHandler, clientSocket, &clientSession);
        // 스레드 비동기 처리
        clientThread.detach();
    }

    return 0;
}
