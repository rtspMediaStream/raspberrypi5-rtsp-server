/**
 * @file OpusEncoder.cpp
 * @brief OpusEncoder 클래스의 구현부
 * @details OpusEncoder 클래스의 멤버 함수를 구현한 소스 파일
 * 
 * Copyright (c) 2024 rtspMediaStream
 * This project is licensed under the MIT License - see the LICENSE file for details
 */

#include "OpusEncoder.h"

/**
 * @brief OpusEncoder 생성자
 * @details 48kHz 샘플링 레이트, 스테레오 채널로 Opus 인코더를 초기화하고
 *          비트레이트를 128kbps로 설정
 * @throw std::runtime_error 인코더 생성 실패 시
 */
OpusEncoder::OpusEncoder()
{
    encoder = opus_encoder_create(OPUS_SAMPLE_RATE, OPUS_CHANNELS, OPUS_APPLICATION_AUDIO, &error);
    if (error != OPUS_OK)
    {
        throw std::runtime_error("Opus 인코더 생성 실패: " + std::string(opus_strerror(error)));
    }

    // 비트레이트 설정 (128kbps)
    opus_encoder_ctl(encoder, OPUS_SET_BITRATE(128000));
}

/**
 * @brief PCM 오디오 데이터를 Opus 포맷으로 인코딩
 * @param pcm_buffer PCM 오디오 데이터 버퍼
 * @param frame_size 프레임 크기 (samples)
 * @param encoded_data 인코딩된 데이터를 저장할 버퍼
 * @return int 인코딩된 데이터의 크기 (bytes)
 * @throw std::runtime_error 인코딩 실패 시
 */
int OpusEncoder::encode(const short *pcm_buffer, int frame_size, unsigned char *encoded_data)
{
    int encoded_bytes = opus_encode(encoder, pcm_buffer, frame_size,
                                    encoded_data, MAX_PACKET_SIZE);
    if (encoded_bytes < 0)
    {
        throw std::runtime_error("Opus 인코딩 실패: " + std::string(opus_strerror(encoded_bytes)));
    }
    return encoded_bytes;
}

/**
 * @brief OpusEncoder 소멸자
 * @details Opus 인코더 리소스 해제
 */
OpusEncoder::~OpusEncoder()
{
    if (encoder)
    {
        opus_encoder_destroy(encoder);
    }
}