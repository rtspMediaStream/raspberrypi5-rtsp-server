/**
 * @file UDPHandler.cpp
 * @brief UDPHandler 클래스의 구현부
 * @details UDPHandler 클래스의 멤버 함수를 구현한 소스 파일
 * 
 * Copyright (c) 2024 rtspMediaStream
 * This project is licensed under the MIT License - see the LICENSE file for details
 */

#include "UDPHandler.h"
#include "ClientSession.h"
#include <string>
#include <cstring>
#include <iostream>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

/**
 * @details 소켓 디스크립터를 초기값 -1로 초기화하여 유효하지 않은 소켓임을 표시
 */
UDPHandler::UDPHandler(std::shared_ptr<ClientSession> client)
    : client(client), rtpSocket(-1), rtcpSocket(-1) {}

/**
 * @details 열려있는 소켓이 있다면(디스크립터가 -1이 아닌 경우) 소켓을 닫고 리소스 해제
 */
UDPHandler::~UDPHandler() {
    if (rtpSocket != -1) close(rtpSocket);
    if (rtcpSocket != -1) close(rtcpSocket);
}

/**
 * @details UDP 소켓 생성 과정:
 *          1. RTP와 RTCP 소켓 각각 생성
 *          2. 주소 구조체 초기화 및 설정
 *             - AF_INET: IPv4 프로토콜
 *             - SOCK_DGRAM: UDP 소켓 타입
 *          3. 클라이언트의 IP 주소와 포트 번호로 주소 구조체 설정
 */
bool UDPHandler::CreateUDPSocket() {
    rtpSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (rtpSocket == -1) {
        std::cerr << "rtp 소켓 생성 실패" << std::endl;
        return false;
    }

    rtcpSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (rtcpSocket == -1) {
        std::cerr << "rtcp 소켓 생성 실패" << std::endl;
        return false;
    }

    memset(&rtpAddr, 0, sizeof(rtpAddr));
    rtpAddr.sin_family = AF_INET;
    rtpAddr.sin_port = htons(client->GetRTPPort());
    inet_pton(AF_INET, client->GetIP().c_str(), &rtpAddr.sin_addr);

    memset(&rtcpAddr, 0, sizeof(rtcpAddr));
    rtcpAddr.sin_family = AF_INET;
    rtcpAddr.sin_port = htons(client->GetRTCPPort());
    inet_pton(AF_INET, client->GetIP().c_str(), &rtcpAddr.sin_addr);

    return true;
}

int& UDPHandler::GetRTPSocket() { return rtpSocket; }
int& UDPHandler::GetRTCPSocket() { return rtcpSocket; }

sockaddr_in& UDPHandler::GetRTPAddr() { return rtpAddr; }
sockaddr_in& UDPHandler::GetRTCPAddr() { return rtcpAddr; }
