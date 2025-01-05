/**
 * @file RTSPServer.h
 * @brief RTSP 서버 메인 클래스 헤더
 * @details RTSP 프로토콜을 지원하는 미디어 스트리밍 서버의 핵심 기능을 제공하는 클래스
 *          - 싱글톤 패턴을 사용한 서버 인스턴스 관리
 *          - 클라이언트 연결 및 세션 관리
 *          - 프로토콜 타입(H264/Opus) 관리
 * 
 * @organization rtspMediaStream
 * @repository https://github.com/rtspMediaStream/raspberrypi5-rtsp-server
 * 
 * Copyright (c) 2024 rtspMediaStream
 * This project is licensed under the MIT License - see the LICENSE file for details
 */

#ifndef __RTSPSERVER_H__
#define __RTSPSERVER_H__
#include <functional>

/**
 * @class FFmpegEncoder
 * @brief FFmpeg 인코딩 기능을 제공하는 클래스
 * @details 카메라 스트리밍을 위한 FFmpeg 기반의 비디오/오디오 인코딩을 담당
 *          실제 구현은 CameraServer/FFmpegEncoder.{h,cpp}에 정의됨
 */
class FFmpegEncoder;

/**
 * @enum Protocol
 * @brief 지원하는 미디어 프로토콜 타입
 */
enum Protocol {
    PROTO_OPUS = 111,   ///< Opus 오디오 코덱 (페이로드 타입: 111)
    PROTO_H264 = 96,    ///< H264 비디오 코덱 (페이로드 타입: 96)
};

class RTSPServer {
private:
    /**
     * @brief 주어진 포트가 권한이 필요한 포트인지 확인
     * @param port 확인할 포트 번호
     * @return bool 권한 필요 여부
     */
    bool isPrivilegedPort(int port);

    /**
     * @brief 프로그램이 루트 권한으로 실행 중인지 확인
     * @return bool 루트 권한 실행 여부
     */
    bool isRunningAsRoot();
    
    Protocol protocol;   ///< 현재 설정된 프로토콜 타입

public:
    /**
     * @brief 싱글톤 패턴을 반환하는 정적 메서드
     * @return RTSPServer& 서버 인스턴스에 대한 참조
     */
    static RTSPServer& getInstance();

    RTSPServer();
    ~RTSPServer();

    /**
     * @brief 서버 스레드를 시작하는 메서드
     * @return int 스레드 시작 결과 (0: 성공, 1: 실패)
     */
    int startServerThread();

    /**
     * @brief 현재 설정된 프로토콜을 반환하는 메서드
     * @return Protocol 현재 프로토콜 타입
     */
    inline Protocol getProtocol() { return protocol; };

    /**
     * @brief 프로토콜 타입을 설정하는 메서드
     * @param _protocol 설정할 프로토콜 타입
     */
    void setProtocol(Protocol _protocol) { protocol = _protocol; };

    std::function<void()> onInitEvent;  ///< 초기화 이벤트 콜백 함수
};

#endif // __RTSPSERVER_H__