/**
 * @file RTPHeader.cpp
 * @brief RTPHeader 클래스의 구현부
 * @details RTPHeader 클래스의 멤버 함수를 구현한 소스 파일
 * 
 * Copyright (c) 2024 rtspMediaStream
 * This project is licensed under the MIT License - see the LICENSE file for details
 */

#include "RTPHeader.hpp"

/**
 * @brief RTPHeader 클래스의 상세 생성자
 * @details 모든 RTP 헤더 필드를 직접 지정하여 초기화하고,
 *          필요한 필드에 대해 네트워크 바이트 순서로 변환한다.
 */
RTPHeader::RTPHeader(const uint8_t  _version,     const uint8_t  _padding,
                     const uint8_t  _extension,   const uint8_t  _csrcCount,
                     const uint8_t  _marker,      const uint8_t  _payloadType,
                     const uint16_t _seq,         const uint32_t _timestamp,
                     const uint32_t _ssrc)
{
    this->version = _version;
    this->padding = _padding;
    this->extension = _extension;
    this->csrcCount = _csrcCount;
    this->payloadType = _payloadType;
    set_marker(_marker);
    this->seq = htons(_seq);
    this->timestamp = htonl(_timestamp);
    this->ssrc = htonl(_ssrc);
}

/**
 * @brief RTPHeader 오버로드된 생성자 - H264 스트리밍용 기본 설정
 * @param _seq 시퀀스 번호
 * @param _timestamp 타임스탬프
 * @param _ssrc 동기화 소스 식별자
 * @details 
 *   - RTP 버전을 2로 설정
 *   - 패딩, 확장, CSRC 카운트를 0으로 초기화
 *   - 마커 비트를 0으로 초기화
 *   - 페이로드 타입을 H264(96)로 설정
 *   - 시퀀스 번호, 타임스탬프, SSRC는 네트워크 바이트 순서로 변환
 */
RTPHeader::RTPHeader(const uint16_t _seq, const uint32_t _timestamp, const uint32_t _ssrc)
{
    this->version = RTP_VERSION;
    this->padding = 0;
    this->extension = 0;
    this->csrcCount = 0;

    this->marker = 0;
    this->payloadType = RTP_PAYLOAD_TYPE_H264;

    this->seq = htons(_seq);
    this->timestamp = htonl(_timestamp);
    this->ssrc = htonl(_ssrc);
}