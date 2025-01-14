/**
 * @file ClientSession.cpp
 * @brief ClientSession 클래스의 구현부
 * @details ClientSession 클래스의 멤버 함수를 구현한 소스 파일
 * 
 * Copyright (c) 2024 rtspMediaStream
 * This project is licensed under the MIT License - see the LICENSE file for details
 */

#include "Global.h"
#include "ClientSession.h"
#include "RequestHandler.h"
#include "UDPHandler.h"
#include "MediaStreamHandler.h"
#include <thread>

/**
 * @details
 *   - 보안과 고유한 세션 ID를 위해 랜덤하게 생성
 *   - SDP 규격에 따라 세션 ID와 세션 버전에 동일한 값을 사용
 *     (RFC 4566 - 5.2. Origin ("o=") 참조)
 *   - TCP 소켓과 IP 주소 설정
 *   - RTP/RTCP 포트를 초기값(-1)으로 설정
 */
ClientSession::ClientSession(const int tcpSocket, const std::string ip) {
    this->id = (int)GetRanNum(16);  // 랜덤한 세션 ID 생성
    this->version = id;             // 세션 버전은 세션 ID와 동일하게 설정
    this->tcpSocket = tcpSocket;
    this->ip = ip;

    this->rtpPort = -1;
    this->rtcpPort = -1;
}

/**
 * @brief 세션 버전을 반환하는 메서드
 * @return int 현재 세션의 버전 번호 (세션 ID와 동일한 값)
 */
int ClientSession::GetVersion() const { return this->version; }