/**
 * @file RTSPServer.cpp
 * @brief RTSPServer 클래스의 구현부
 * @details RTSP 서버의 초기화, 실행, 종료 처리를 구현
 * 
 * Copyright (c) 2024 rtspMediaStream
 * This project is licensed under the MIT License - see the LICENSE file for details
 */

#include "RTSPServer.h"
#include "Global.h"
#include "ClientSession.h"
#include "TCPHandler.h"
#include "UDPHandler.h"
#include "RequestHandler.h"
#include "MediaStreamHandler.h"

#include <string>
#include <thread>
#include <iostream>
#include <csignal>
#include <vector>
#include <fstream>
#include <iomanip>
#include <memory>
using namespace std;

/**
 * @details 서버 인스턴스 초기화
 */
RTSPServer::RTSPServer()
{
    
}

/**
 * @details 서버 종료 시 리소스 정리 및 클라이언트 연결 종료 처리
 */
RTSPServer::~RTSPServer()
{
    std::cout << "Interrupt signal (" << signal << ") received. Shutting down..." << std::endl;
    TCPHandler::GetInstance().CloseClientConnection();
}

/**
 * @details 서버 스레드 시작 프로세스:
 *          1. 권한 검사 (privileged port 사용 시)
 *          2. 클라이언트 연결 수신 대기
 *          3. 새 클라이언트 연결 시 세션 생성
 *          4. 요청 처리 스레드 시작
 */
int RTSPServer::startServerThread()
{
    if(isPrivilegedPort(g_serverRtpPort) && !isRunningAsRoot()) {
        std::cerr << "Error: Program must be run as root to bind to privileged ports.\n";
        return 1;
    };

    std::cout << "Start RTSP server " << std::endl;

    std::thread ([]()->void{
        while(true){
            std::string newIp;
            int newClient = TCPHandler::GetInstance().AcceptClientConnection(newIp);
            if(newClient == -1){
                break;
            }else {
                std::cout << "Client Ip:" << newIp << " connected." << std::endl;
            }
            ClientSession* clientSession = new ClientSession(newClient, newIp);

            RequestHandler requestHandler(clientSession);
            requestHandler.StartThread();
        }
        std::cout << "Stop RTSP server" << std::endl;
    }).detach();
 
    return 0;
}

/**
 * @details 1024 이하의 포트는 privileged port로 간주
 */
bool RTSPServer::isPrivilegedPort(int port) {
    return port <= 1024;
}

/**
 * @details 프로세스의 실효 사용자 ID가 0(root)인지 확인
 */
bool RTSPServer::isRunningAsRoot() {
    return getuid() == 0;
}