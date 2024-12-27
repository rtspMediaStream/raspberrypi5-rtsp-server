/**
 * @file MediaStreamHandler.h
 * @brief 미디어 스트림 처리 클래스 헤더
 * @details RTSP 프로토콜을 통한 미디어 스트리밍을 처리하는 클래스로,
 *          비디오/오디오 스트림의 상태 관리 및 RTP 패킷 전송을 담당
 * 
 * @organization rtspMediaStream
 * @repository https://github.com/rtspMediaStream/raspberrypi5-rtsp-server
 * 
 * Copyright (c) 2024 rtspMediaStream
 * This project is licensed under the MIT License - see the LICENSE file for details
 */

#ifndef RTSP_MEDIASTREAMHANDLER_H
#define RTSP_MEDIASTREAMHANDLER_H

#include <atomic>
#include <alsa/asoundlib.h>
#include <condition_variable>

/**
 * @enum MediaStreamState
 * @brief 미디어 스트림의 상태를 나타내는 열거형
 */
enum MediaStreamState{
    eMediaStream_Init,     ///< 초기화 상태
    eMediaStream_Play,     ///< 재생 상태
    eMediaStream_Pause,    ///< 일시 정지 상태
    eMediaStream_Teardown, ///< 종료 상태
};

/// @class RtpPacket
/// @brief RTP 패킷 관리를 위한 클래스
/// @details 실제 구현은 rtp_packet.hpp에 정의되어 있으며,
///          RTP 패킷의 생성, 관리, 전송 클래스
class RtpPacket;

/**
 * @class MediaStreamHandler
 * @brief 미디어 스트리밍 처리를 담당하는 클래스
 * @details 미디어 스트림의 상태 관리, RTP 패킷 생성 및 전송,
 *          스트리밍 제어 기능을 제공하는 클래스
 */
class MediaStreamHandler {
public:
    UDPHandler* udpHandler; ///< UDP 통신을 위한 핸들러  

    /**
     * @brief 생성자 - 스트림 핸들러 초기화
     */
    MediaStreamHandler();

    /**
     * @brief 미디어 스트림을 처리하는 메인 메서드
     * @details 스트림의 상태에 따라 미디어 데이터를 처리하고 전송
     */
    void HandleMediaStream();

    /**
     * @brief 스트리밍 명령을 설정하는 메서드
     * @param cmd 설정할 명령 (PLAY, PAUSE, TEARDOWN)
     */
    void SetCmd(const std::string& cmd);

private:
    bool threadRun = true;              ///< 스트림 실행 상태
    MediaStreamState streamState;       ///< 현재 스트림 상태
    std::mutex streamMutex;             ///< 스트림 동기화를 위한 뮤텍스
    std::condition_variable condition;  ///< 스트림 상태 제어을 위한 조건 변수

    /**
     * @brief 오디오 스트림을 처리하는 메서드
     * @param payload 전송할 페이로드 데이터
     * @param payloadSize 전송할 페이로드 데이터 크기
     * @param rtpPacket RTP 패킷 객체
     * @param timeStamp RTP 패킷의 타임스탬프
     * @details 오디오 데이터를 MTU 크기에 따라 단일 또는 분할하여 RTP 패킷으로 생성하고 전송
     */
    void SendFragmentedRTPPackets(unsigned char* payload, size_t payloadSize, RtpPacket& rtpPacket, const uint32_t timeStamp);
};

#endif //RTSP_MEDIASTREAMHANDLER_H
