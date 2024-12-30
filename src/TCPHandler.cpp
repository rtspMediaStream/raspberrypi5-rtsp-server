/**
 * @file TCPHandler.cpp
 * @brief TCPHandler 클래스의 구현부
 * @details TCPHandler 클래스의 멤버 함수를 구현한 소스 파일
 * 
 * Copyright (c) 2024 rtspMediaStream
 * This project is licensed under the MIT License - see the LICENSE file for details
 */

#include "TCPHandler.h"
#include "Global.h"

#include <string>
#include <cstring>
#include <iostream>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

/**
 * @brief TCPHandler 클래스의 생성자
 * @details TCP 포트를 설정하고 TCP 소켓을 생성
 */
TCPHandler::TCPHandler(): tcpPort(g_serverRtpPort) {
    CreateTCPSocket();
}

/**
 * @brief TCPHandler 클래스의 소멸자
 */
TCPHandler::~TCPHandler() {}

/**
 * @brief TCP 소켓을 생성하고 초기화하는 메서드
 * @details 
 *   - TCP 소켓 생성
 *   - SO_REUSEADDR 옵션 설정
 *   - 주소 바인딩
 *   - 리스닝 상태로 전환
 * @throw std::runtime_error 소켓 생성, 바인딩 또는 리스닝 실패 시
 */
void TCPHandler::CreateTCPSocket() {
    tcpSocket = socket(AF_INET, SOCK_STREAM, 0);
    int option = 1;          // SO_REUSEADDR 의 옵션 값을 TRUE 로
    setsockopt( tcpSocket, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option) );
    if (tcpSocket == -1) {
        std::cerr << "Error: fail to create TCP socket" << std::endl;
        exit(1);
    }

    memset(&tcpAddr, 0, sizeof(tcpAddr));
    tcpAddr.sin_family = AF_INET;
    tcpAddr.sin_addr.s_addr = INADDR_ANY;
    tcpAddr.sin_port = htons(tcpPort);

    if (bind(tcpSocket, (struct sockaddr*)&tcpAddr, sizeof(tcpAddr)) == -1) {
        std::cerr << "Error: fail to bind TCP socket" << std::endl;
        exit(1);
    }

    if (listen(tcpSocket, 10) == -1) {
        std::cerr << "Error: fail to listen TCP socket" << std::endl;
        exit(1);
    }
}

/**
 * @brief 클라이언트 연결을 수락하는 메서드
 * @param _clientIp [out] 연결된 클라이언트의 IP 주소
 * @return int 생성된 클라이언트 소켓 디스크립터 (-1: 실패)
 * @details 클라이언트의 연결 요청을 수락하고 IP 주소를 획득
 */
int TCPHandler::AcceptClientConnection(std::string &_clientIp) {
    socklen_t clientAddrLen = sizeof(tcpAddr);
    int clientSocket = accept(tcpSocket, (sockaddr*)&tcpAddr, &clientAddrLen);

    if (clientSocket == -1) {
        std::cerr << "Error: fail to connect client" << std::endl;
        return -1;
    }

    char clientIP[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &tcpAddr.sin_addr, clientIP, INET_ADDRSTRLEN);
    
    _clientIp = clientIP;
    return clientSocket;
}

/**
 * @brief 클라이언트 연결을 종료하는 메서드
 * @details TCP 소켓을 닫고 연결을 종료
 */
void TCPHandler::CloseClientConnection() {
    close(tcpSocket);
}

/**
 * @brief RTSP 요청을 수신하는 메서드
 * @param clientSocket 클라이언트 소켓 디스크립터
 * @return std::string 수신된 RTSP 요청 메시지
 * @details 클라이언트로부터 RTSP 요청 메시지를 수신
 */
std::string TCPHandler::ReceiveRTSPRequest(int clientSocket) {
    char buffer[1024] = {0,};
    memset(buffer, 0, sizeof(buffer));

    int receivedBytes = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);

    if (receivedBytes <= 0) {
        std::cerr << "Error: fail to recv RTSP request" << std::endl;
        return nullptr;
    }

    std::cout << "rtsp 요청:\n" << buffer << std::endl;
    return {buffer};
}

/**
 * @brief RTSP 응답을 전송하는 메서드
 * @param clientSocket 클라이언트 소켓 디스크립터
 * @param response 전송할 RTSP 응답 메시지
 * @details 클라이언트에게 RTSP 응답 메시지를 전송
 */
void TCPHandler::SendRTSPResponse(int clientSocket, std::string& response) {
    int sentBytes = send(clientSocket, response.c_str(), response.size(), 0);
    if (sentBytes == -1) {
        std::cerr << "Error: fail to send RTSP response" << std::endl;
    }
}

/**
 * @brief TCP 소켓을 반환하는 메서드
 * @return int& TCP 소켓 디스크립터에 대한 참조
 */
int& TCPHandler::GetTCPSocket() { return tcpSocket; }

/**
 * @brief TCP 주소 구조체를 반환하는 메서드
 * @return sockaddr_in& TCP 주소 구조체에 대한 참조
 */
sockaddr_in& TCPHandler::GetTCPAddr() { return tcpAddr; }
