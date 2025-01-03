/**
 * @file RequestHandler.h
 * @brief RTSP 요청 처리 클래스 헤더
 * @details RTSP 프로토콜의 각 메서드(OPTIONS, DESCRIBE, SETUP, PLAY, PAUSE, TEARDOWN)에 대한
 *          요청을 처리하고 적절한 응답을 생성하는 클래스
 * 
 * @organization rtspMediaStream
 * @repository https://github.com/rtspMediaStream/raspberrypi5-rtsp-server
 * 
 * Copyright (c) 2024 rtspMediaStream
 * This project is licensed under the MIT License - see the LICENSE file for details
 */

#ifndef RTSP_REQUESTHANDLER_H
#define RTSP_REQUESTHANDLER_H

#include <memory>
#include <string>
class ClientSession;
class MediaStreamHandler;

/**
 * @class RequestHandler
 * @brief RTSP 요청 처리 클래스
 * @details 클라이언트로부터 받은 RTSP 요청을 파싱하고 처리하며,
 *          적절한 응답(OPTIONS, DESCRIBE, SETUP, PLAY, PAUSE, TEARDOWN)을 생성하는 기능을 제공한다.
 */

class RequestHandler {
public:
    /**
     * @brief RequestHandler 생성자
     * @param session 클라이언트 세션 객체
     */
    RequestHandler(ClientSession* session);

    /**
     * @brief 함수 호출 연산자 오버로딩
     * @details 요청 처리를 위한 HandleRequest 메서드 호출
     */
    void operator()() { HandleRequest(); }

    /**
     * @brief RTSP 요청을 처리하는 메인 메서드
     */
    void HandleRequest();

    /**
     * @brief 요청 처리를 위한 스레드를 시작하는 메서드
     */
    void StartThread();

private:
    std::shared_ptr<ClientSession> session; ///< 클라이언트 세션 객체

    MediaStreamHandler *mediaStreamHandler; ///< 미디어 스트림 핸들러 처리 객체

    /**
     * @brief RTSP 메서드를 파싱하는 메서드
     * @param request RTSP 요청 문자열
     * @return std::string 파싱된 RTSP 메서드
     */
    std::string ParseMethod(const std::string& request);

    /**
     * @brief CSeq 값을 파싱하는 메서드
     * @param request RTSP 요청 문자열
     * @return int 파싱된 CSeq 값 (-1: 파싱 실패)
     */
    int ParseCSeq(const std::string& request);

    /**
     * @brief RTP/RTCP 포트를 파싱하는 메서드
     * @param request RTSP 요청 문자열
     * @return std::pair<int, int> RTP와 RTCP 포트 번호 쌍
     */
    std::pair<int, int> ParsePorts(const std::string& request);

    /**
     * @brief Accept 헤더를 파싱하는 메서드
     * @param request RTSP 요청 문자열
     * @return bool SDP 수락 여부
     */
    bool ParseAccept(const std::string& request);

    /**
     * @brief OPTIONS 요청을 처리하는 메서드
     * @param cseq 요청의 CSeq 값
     */
    void HandleOptionsRequest(const int cseq);

    /**
     * @brief DESCRIBE 요청을 처리하는 메서드
     * @param request RTSP 요청 문자열
     * @param cseq 요청의 CSeq 값
     */
    void HandleDescribeRequest(const std::string& request, const int cseq);

    /**
     * @brief SETUP 요청을 처리하는 메서드
     * @param request RTSP 요청 문자열
     * @param cseq 요청의 CSeq 값
     */
    void HandleSetupRequest(const std::string& request, const int cseq);

    /**
     * @brief PLAY 요청을 처리하는 메서드
     * @param cseq 요청의 CSeq 값
     */
    void HandlePlayRequest(int cseq);

    /**
     * @brief PAUSE 요청을 처리하는 메서드
     * @param cseq 요청의 CSeq 값
     */
    void HandlePauseRequest(int cseq);

    /**
     * @brief TEARDOWN 요청을 처리하는 메서드
     * @param cseq 요청의 CSeq 값
     */
    void HandleTeardownRequest(int cseq);
};

#endif //RTSP_REQUESTHANDLER_H