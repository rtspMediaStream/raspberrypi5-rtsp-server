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
 * @brief ClientSession 클래스의 생성자
 * @param tcpSocket 클라이언트와 연결된 TCP 소켓 디스크립터
 * @param ip 클라이언트의 IP 주소
 * @details
 *   - 보안과 고유한 세션 ID를 위해 랜덤하게 생성
 *   - 세션 버전 초기화
 *   - TCP 소켓과 IP 주소 설정
 *   - RTP/RTCP 포트를 초기값(-1)으로 설정
 */
ClientSession::ClientSession(const int tcpSocket, const std::string ip) {
    this->id = (int)GetRanNum(16);
    this->version = id;
    this->tcpSocket = tcpSocket;
    this->ip = ip;

    this->rtpPort = -1;
    this->rtcpPort = -1;
}

/**
 * @brief 세션 버전을 반환하는 메서드
 * @return int 현재 세션의 버전 번호
 */
int ClientSession::GetVersion() const { return this->version; }