/**
 * @file ClientSession.h
 * @brief RTSP 클라이언트 세션 관리 클래스 헤더
 * @details RTSP 서버에 연결된 각 클라이언트의 세션 정보와 상태를 관리하는 클래스
 *          - 클라이언트 식별을 위한 세션 ID 관리
 *          - TCP/UDP 소켓 및 포트 관리
 *          - 클라이언트 IP 주소 관리
 * 
 * @organization rtspMediaStream
 * @repository https://github.com/rtspMediaStream/raspberrypi5-rtsp-server
 * 
 * Copyright (c) 2024 rtspMediaStream
 * This project is licensed under the MIT License - see the LICENSE file for details
 */
#ifndef RTSP_CLIENTSESSION_H
#define RTSP_CLIENTSESSION_H

#include <map>
#include <queue>
#include <mutex>
#include <chrono>
#include <string>
#include <iostream>
#include <memory>

class RequestHandler;
class UDPHandler;
class MediaStreamHandler;

/**
 * @class ClientSession
 * @brief RTSP 클라이언트 세션을 관리하는 클래스
 * @details 각 클라이언트 연결에 대한 세션 정보를 저장하고 관리하는 클래스로,
 *          클라이언트의 연결 정보, 스트리밍 상태, 미디어 전송을 위한 소켓 정보 등을 관리한다.
 */
class ClientSession {
public:
    /** 
     * @brief 클라이언트 세션 생성자 - 새로운 클라이언트 세션 초기화
     * @param tcpSocket 클라이언트와 연결된 TCP 소켓
     * @param ip 클라이언트 IP 주소
     */
    ClientSession(const int tcpSocket, const std::string ip);
    
    /** 
     * @brief 클라이언트 세션 ID를 반환하는 메서드
     * @return int 클라이언트 세션 ID
     */
    int GetSessionId() const;

    /**
     * @brief 클라이언트 세션 버전 정보를 반환하는 메서드
     * @return int 클라이언트 세션 버전 정보
     */
    int GetVersion() const;

    /**
     * @brief 클라이언트 ID를 반환하는 메서드
     * @return int 랜덤하게 생성된 클라이언트의 고유 ID
     */
    inline int GetID() { return this->id; };

    /**
     * @brief 클라이언트 세션의 TCP 소켓 정보를 반환하는 메서드
     * @return int 클라이언트 세션의 TCP 소켓 디스크립터
     */
    inline int GetTCPSocket() { return this->tcpSocket; };

    /**
     * @brief 클라이언트 세션의 RTP 포트 번호를 반환하는 메서드
     * @return int 클라이언트 세션의 RTP 포트 번호
     */
    inline int GetRTPPort() { return this->rtpPort; };

    /**
     * @brief 클라이언트 세션의 RTCP 포트 번호를 반환하는 메서드
     * @return int 클라이언트 세션의 RTCP 포트 번호
     */
    inline int GetRTCPPort() { return this->rtcpPort; };

    /**
     * @brief 클라이언트 세션의 IP 주소를 반환하는 메서드
     * @return std::string 클라이언트 세션의 IP 주소
     */
    inline std::string GetIP() { return this->ip; };

    /**
     * @brief RTP 포트 번호를 설정하는 메서드
     * @param rtpPort를 설정할 RTP 포트 번호
     */
    inline void SetRTPPort(int rtpPort) { this->rtpPort = rtpPort; };

    /**
     * @brief RTCP 포트 번호를 설정하는 메서드
     * @param rtcpPort를 설정할 RTCP 포트 번호
     */
    inline void SetRTCPPort(int rtcpPort) { this->rtcpPort = rtcpPort; };

private:
    int id;         ///< 클라이언트 세션 ID
    int version;    ///< 클라이언트 세션 버전 정보
    int tcpSocket;  ///< TCP 소켓 디스크립터
    int rtpPort;    ///< RTP 스트리밍을 위한 포트 번호
    int rtcpPort;   ///< RTCP 제어를 위한 포트 번호
    std::string ip; ///< 클라이언트 IP 주소
    
    RequestHandler* requestHandler; ///< 클라이언트 요청 처리를 위한 핸들러
};

#endif //RTSP_CLIENTSESSION_H