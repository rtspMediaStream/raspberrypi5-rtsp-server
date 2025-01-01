/**
 * @file TCPHandler.h
 * @brief TCP 연결 및 RTSP 메시지 처리를 위한 싱글톤 클래스 헤더
 * @details TCP 소켓을 생성하고 클라이언트 연결을 관리하며,
 *          RTSP 프로토콜에 따른 메시지 송수신을 처리하는 기능을 제공하는 클래스
 *          - TCP 소켓 생성 및 초기화
 *          - 클라이언트 연결 수락 및 관리
 *          - RTSP 요청/응답 메시지 처리
 * 
 * @organization rtspMediaStream
 * @repository https://github.com/rtspMediaStream/raspberrypi5-rtsp-server
 * 
 * Copyright (c) 2024 rtspMediaStream
 * This project is licensed under the MIT License - see the LICENSE file for details
 */

#ifndef RTSP_TCPHANDLER_H
#define RTSP_TCPHANDLER_H

#include <string>
#include <arpa/inet.h>
#include <unordered_map>
#include <vector>

/**
 * @class TCPHandler
 * @brief TCP 연결 관리 및 RTSP 메시지 처리를 위한 싱글톤 클래스
 * @details TCP 소켓을 생성하고 클라이언트 연결을 관리하며,
 *          RTSP 프로토콜에 따른 메시지 송수신을 처리하는 기능을 제공한다.
 */
class TCPHandler {
public:
    /** 
     * @brief 복사 생성자 삭제 (싱글톤 패턴)
     */
    TCPHandler(const TCPHandler&) = delete;

    /** 
     * @brief 대입 연산자 삭제 (싱글톤 패턴)
     */
    TCPHandler& operator=(const TCPHandler&) = delete;

    /**
     * @brief 싱글톤 인스턴스를 반환하는 정적 메서드
     * @return TCPHandler& 싱글톤 인스턴스에 대한 참조
     */
    static TCPHandler& GetInstance() {
        static TCPHandler instance;
        return instance;
    };

    /**
     * @brief TCP 소켓을 생성하고 초기화하는 메서드
     * @details 소켓 생성, 옵션 설정, 바인딩, 리스닝 상태 설정
     * @throw std::runtime_error 소켓 생성 또는 초기화 실패 시
     */
    void CreateTCPSocket();

    /**
     * @brief 클라이언트 연결을 수락하는 메서드
     * @param _clientIp [out] 연결된 클라이언트의 IP 주소
     * @return int 생성된 클라이언트 소켓 디스크립터
     * @throw std::runtime_error 연결 수락 실패 시
     */
    int AcceptClientConnection(std::string &_clientIp);

    /** 
     * @brief 클라이언트 연결을 종료하는 메서드
     */
    void CloseClientConnection();

    /** 
     * @brief RTSP 요청 메시지를 수신하는 메서드
     * @param sessionId 클라이언트 세션 ID
     * @return std::string 수신된 RTSP 요청 메시지
     */
    std::string ReceiveRTSPRequest(int sessionId);

    /**
     * @brief RTSP 응답을 전송하는 메서드
     * @param clientSocket 클라이언트 소켓 디스크립터
     * @param response 전송할 RTSP 응답 메시지
     */
    void SendRTSPResponse(int clientSocket, std::string& response);

    /**
     * @brief TCP 소켓을 반환하는 메서드
     * @return int& TCP 소켓 디스크립터에 대한 참조
     */
    int& GetTCPSocket();

    /**
     * @brief TCP 주소 구조체를 반환하는 메서드
     * @return sockaddr_in& TCP 주소 구조체에 대한 참조
     */
    sockaddr_in& GetTCPAddr();


private:
    /**
     * @brief 생성자 - TCP 소켓 초기화
     */
    TCPHandler();

    /**
     * @brief 소멸자 - TCP 소켓 닫기
     */
    ~TCPHandler();

    int tcpPort;                                ///< TCP 포트 번호
    int tcpSocket;                              ///< TCP 소켓 디스크립터
    sockaddr_in tcpAddr;                        ///< TCP 주소 구조체
    std::unordered_map<int, int> socketTable;   ///< 클라이언트 소켓 테이블
};

#endif //RTSP_TCPHANDLER_H