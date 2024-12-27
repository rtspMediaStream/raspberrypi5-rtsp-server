/**
 * @file OpusEncoder.h
 * @brief Opus 오디오 인코딩 클래스 헤더
 * @details Opus 코덱을 사용하여 오디오 데이터를 인코딩하는 기능을 제공하는 클래스
 * 
 * @organization rtspMediaStream
 * @repository https://github.com/rtspMediaStream/raspberrypi5-rtsp-server
 * 
 * Copyright (c) 2024 rtspMediaStream
 * This project is licensed under the MIT License - see the LICENSE file for details
 */

#include <opus/opus.h>
#include <iostream>

// Opus 인코더 설정 상수
const static int OPUS_SAMPLE_RATE = 48000;                  ///< 샘플링 레이트 (Hz)
const static int OPUS_CHANNELS = 2;                         ///< 오디오 채널 수 (2: 스테레오)
const static int OPUS_FRAME_SIZE = 960;                     ///< 프레임 크기 (20ms)
const static int APPLICATION = OPUS_APPLICATION_AUDIO;      ///< Opus 애플리케이션 모드 (음악 및 음성)
const static int MAX_PACKET_SIZE = 1500;                    ///< 최대 패킷 크기 (bytes)

/**
 * @class OpusEncoder
 * @brief Opus 오디오 인코딩 클래스
 * @details PCM 오디오 데이터를 Opus 포맷으로 인코딩하는 기능을 제공하는 클래스
 */
class OpusEncoder {
private:
    OpusEncoder* encoder;                           ///< Opus 인코더 인스턴스
    unsigned char encoded_buffer[MAX_PACKET_SIZE];  ///< 인코딩된 데이터 버퍼 [최대 패킷 크기]
    int error;                                      ///< 인코딩 에러 코드

public:
    /**
     * @brief 생성자 - Opus 인코더 초기화
     * @throw std::runtime_error 인코더 생성 실패 시
     */
    OpusEncoder();

    /**
     * @brief PCM 데이터를 Opus 포맷으로 인코딩
     * @param pcm_buffer PCM 오디오 데이터 버퍼
     * @param frame_size 프레임 크기 (samples)
     * @param encoded_data 인코딩된 데이터를 저장할 버퍼
     * @return int 인코딩된 데이터의 크기 (bytes)
     * @throw std::runtime_error 인코딩 실패 시
     */
    int encode(const short* pcm_buffer, int frame_size, unsigned char* encoded_data);

    /**
     * @brief 소멸자 - 인코더 리소스 해제
     */
    ~OpusEncoder();
};