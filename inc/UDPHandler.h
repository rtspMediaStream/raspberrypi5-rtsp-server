/**
 * @file UDPHandler.h
 * @brief UDP 소켓 관리 클래스 헤더
 * @details RTP/RTCP 스트리밍을 위한 UDP 소켓을 관리하는 클래스
 *          - RTP 소켓 생성 및 관리
 *          - RTCP 소켓 생성 및 관리
 *          - 클라이언트 주소 관리
 * 
 * @organization rtspMediaStream
 * @repository https://github.com/rtspMediaStream/raspberrypi5-rtsp-server
 * 
 * Copyright (c) 2024 rtspMediaStream
 * This project is licensed under the MIT License - see the LICENSE file for details
 */

#ifndef RTSP_UDPHANDLER_H
#define RTSP_UDPHANDLER_H

#include <string>
#include <arpa/inet.h>
#include <memory>

class ClientSession;

/**
 * @class UDPHandler
 * @brief RTP/RTCP를 위한 UDP 소켓 관리 클래스
 * @details 클라이언트와의 미디어 스트리밍을 위한 UDP 소켓을 관리하며,
 *          RTP/RTCP 프로토콜에 필요한 소켓과 주소 정보를 제공한다.
 */
class UDPHandler {
public:
    /**
     * @brief UDP 핸들러 생성자
     * @param client 클라이언트 세션 객체
     */
    UDPHandler(std::shared_ptr<ClientSession> client);

    /**
     * @brief 소멸자 - 소켓 리소스 해제
     */
    ~UDPHandler();

    /**
     * @brief RTP와 RTCP UDP 소켓을 생성하는 메서드
     * @return bool 소켓 생성 성공 여부
     * @details 클라이언트의 RTP/RTCP 포트로 연결할 UDP 소켓을 생성
     */
    bool CreateUDPSocket();

    /**
     * @brief RTP 소켓을 반환하는 메서드
     * @return int& RTP 소켓 디스크립터에 대한 참조
     */
    int& GetRTPSocket();

    /**
     * @brief RTCP 소켓을 반환하는 메서드
     * @return int& RTCP 소켓 디스크립터에 대한 참조
     */
    int& GetRTCPSocket();

    /**
     * @brief RTP 주소를 반환하는 메서드
     * @return sockaddr_in& RTP 주소 구조체에 대한 참조
     */
    sockaddr_in& GetRTPAddr();

    /**
     * @brief RTCP 주소를 반환하는 메서드
     * @return sockaddr_in& RTCP 주소 구조체에 대한 참조
     */
    sockaddr_in& GetRTCPAddr();


private:
    std::shared_ptr<ClientSession> client; ///< 클라이언트 세션 객체
    int rtpSocket;                         ///< RTP 소켓 디스크립터
    int rtcpSocket;                        ///< RTCP 소켓 디스크립터
    sockaddr_in rtpAddr;                   ///< RTP 주소 구조체
    sockaddr_in rtcpAddr;                  ///< RTCP 주소 구조체
};

#endif //RTSP_UDPHANDLER_H