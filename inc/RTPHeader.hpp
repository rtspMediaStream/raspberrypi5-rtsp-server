/**
 * @file RTPHeader.hpp
 * @brief RTP 헤더 정의 및 처리 클래스 헤더
 * @details RTP(Real-time Transport Protocol) 헤더의 구조체를 정의하고 처리하는 클래스
 *          - RTP 헤더 구조 정의 및 비트필드 처리
 *          - 네트워크 바이트 오더 변환
 *          - MTU 기반 패킷 크기 제한 처리
 * 
 * @organization rtspMediaStream
 * @repository https://github.com/rtspMediaStream/raspberrypi5-rtsp-server
 * 
 * Copyright (c) 2024 rtspMediaStream
 * This project is licensed under the MIT License - see the LICENSE file for details
 */

#pragma once

#include <cstddef>
#include <cstdint>

#include <arpa/inet.h>

/// RTP/UDP 패킷 관련 상수 정의
constexpr int64_t IP_V4_HEADER_SIZE = 20;        ///< IPv4 헤더 크기
constexpr int64_t UDP_HEADER_SIZE = 8;           ///< UDP 헤더 크기
constexpr int64_t RTP_HEADER_SIZE = 12;          ///< RTP 헤더 크기
constexpr int64_t RTP_VERSION = 2;               ///< RTP 버전
constexpr int64_t RTP_PAYLOAD_TYPE_H264 = 96;    ///< H264 페이로드 타입
constexpr int64_t FU_SIZE = 2;                   ///< Fragmentation Unit 크기

/// 최대 패킷 크기 관련 상수
constexpr int64_t MAX_UDP_PACKET_SIZE = 65535;   ///< UDP 최대 패킷 크기
constexpr int64_t MAX_RTP_DATA_SIZE = MAX_UDP_PACKET_SIZE - IP_V4_HEADER_SIZE
                                     - UDP_HEADER_SIZE - RTP_HEADER_SIZE - FU_SIZE;  ///< RTP 최대 데이터 크기
constexpr int64_t MAX_RTP_PACKET_LEN = MAX_RTP_DATA_SIZE + RTP_HEADER_SIZE + FU_SIZE;  ///< RTP 최대 패킷 길이


#pragma pack(1) ///< 1바이트 정렬로 패딩 없이 메모리에 헤더 구조체를 배치

/**
 * @class RTPHeader
 * @brief RTP 헤더를 정의하고 처리하는 클래스
 * @details RTP 헤더의 각 필드를 비트 단위로 정의하고,
 *          네트워크 바이트 오더 변환 및 헤더 필드 접근을 처리한다.
 */
class RTPHeader
{
public:
    /**
     * @brief RTP 헤더 생성자 - 모든 필드 직접 지정
     * @param _version RTP 버전
     * @param _padding 패딩 비트
     * @param _extension 확장 비트
     * @param _csrcCount CSRC 카운트
     * @param _marker 마커 비트
     * @param _payloadType 페이로드 타입
     * @param _seq 시퀀스 번호
     * @param _timestamp 타임스탬프
     * @param _ssrc 동기화 소스 식별자
     */
    RTPHeader(uint8_t _version,     uint8_t _padding,
              uint8_t _extension,   uint8_t _csrcCount,
              uint8_t _marker,      uint8_t _payloadType,
              uint16_t _seq,        uint32_t _timestamp,
              uint32_t _ssrc);

    /**
     * @brief RTP 헤더 생성자 - 기본값으로 초기화
     * @param _seq 시퀀스 번호
     * @param _timestamp 타임스탬프
     * @param _ssrc 동기화 소스 식별자
     */
    RTPHeader(uint16_t _seq, uint32_t _timestamp, uint32_t _ssrc);

    /**
     * @brief RTP 헤더 복사 생성자
     */
    RTPHeader(const RTPHeader &) = default;

    /**
     * @brief RTP 헤더 소멸자
     */
    ~RTPHeader() = default;

    /**
     * @brief 타임스탬프 설정
     * @param _newtimestamp 새로운 타임스탬프 값
     */
    inline void set_timestamp(const uint32_t _newtimestamp) { this->timestamp = htonl(_newtimestamp); };

    /**
     * @brief SSRC 설정
     * @param SSRC 새로운 SSRC 값
     */
    inline void set_ssrc(const uint32_t SSRC) { this->ssrc = htonl(SSRC); };

    /**
     * @brief 시퀀스 번호 설정
     * @param _seq 새로운 시퀀스 번호
     */
    inline void set_seq(const uint32_t _seq) { this->seq = htons(_seq); };

    /**
     * @brief 마커 비트 설정 (RTP 헤더의 M 비트)
     * @param _marker 마커 비트 값 (true: 설정, false: 해제)
     * @details 
     *   RTP 패킷의 마커 비트(M bit)를 설정하는 메서드입니다.
     *   - true로 설정 시: marker |= 0x01 (비트 OR 연산으로 마지막 비트를 1로 설정)
     *   - false로 설정 시: marker &= ~0x01 (비트 AND 연산으로 마지막 비트를 0으로 설정)
     *   
     *   마커 비트는 페이로드 특정적인 의미를 가지며, H.264 비디오의 경우:
     *   - 프레임의 마지막 패킷임을 나타낼 때 1로 설정
     *   - 프레임의 중간 패킷들에서는 0으로 설정
     */
    inline void set_marker(const bool _marker){ _marker? marker |= 0x01 : marker &= ~0x01; };

    /**
     * @brief 페이로드 타입 설정
     * @param _payloadType 새로운 페이로드 타입
     */
    inline void set_payloadType(const uint8_t _payloadType) { payloadType = _payloadType; };

    /**
     * @brief 헤더 포인터 반환
     * @return void* 헤더의 메모리 주소
     */
    inline void *get_header() const { return (void *)this; };

    /**
     * @brief 타임스탬프 반환 (네트워크 바이트 순서 변환 처리)
     * @return uint32_t 현재 타임스탬프
     */
    inline uint32_t get_timestamp() const { return ntohl(this->timestamp); };

    /**
     * @brief 시퀀스 번호 반환 (네트워크 바이트 순서 변환 처리)
     * @return uint32_t 현재 시퀀스 번호
     */
    inline uint32_t get_seq() const { return ntohs(this->seq); };

private:
    uint8_t csrcCount : 4;    ///< CSRC 카운트 (4비트)
    uint8_t extension : 1;    ///< 확장 비트 (1비트)
    uint8_t padding : 1;      ///< 패딩 비트 (1비트)
    uint8_t version : 2;      ///< 버전 정보 (2비트)

    uint8_t payloadType : 7;  ///< 페이로드 타입 (7비트)
    uint8_t marker : 1;       ///< 마커 비트 (1비트)

    uint16_t seq;             ///< 시퀀스 번호
    uint32_t timestamp;       ///< 타임스탬프
    uint32_t ssrc;           ///< 동기화 소스 식별자
};

#pragma pack() ///< 패킹 설정을 원래대로 복구
