/**
 * @file RTCPPacket.hpp
 * @brief RTCP 패킷 데이터 및 전송 처리 클래스 헤더
 * @details RTCP(RTP Control Protocol) 패킷의 구조체를 정의하고 전송을 처리하는 클래스
 *          - RTCP Sender Report 패킷 구조 정의
 *          - 빅엔디안, 리틀엔디안을 위한 네트워크 바이트 처리
 *          - UDP 소켓을 통한 패킷 전송
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
#include "RTSPServer.h"

#pragma pack(1)  ///< 1바이트 정렬로 패딩 없이 메모리에 패킷 구조체 배치

/**
 * @class RTCPPacket
 * @brief RTCP 패킷을 생성하고 전송하는 클래스
 * @details RTCP Sender Report 패킷의 구조를 정의하고, 
 *          네트워크 바이트 오더 변환과 UDP 소켓을 통한 전송을 처리한다.
 */
class RTCPPacket
{
public:
    /**
     * @brief RTCP 패킷 생성자
     * @param timestamp RTP 타임스탬프
     * @param packetCount 전송된 패킷 수
     * @param octetCount 전송된 총 바이트 수
     * @param payloadType 페이로드 타입 (H264 또는 OPUS)
     */
    RTCPPacket(const unsigned int timestamp, unsigned int packetCount, unsigned int octetCount, Protocol payloadType);

    /**
     * @brief RTCP 패킷을 UDP 소켓으로 전송하는 메서드
     * @param sockfd UDP 소켓 디스크립터
     * @param _bufferLen 전송할 버퍼의 크기
     * @param flags 전송 옵션 플래그
     * @param to 수신자 주소 정보
     * @return int64_t 전송된 바이트 수
     */
    int64_t rtcp_sendto(int sockfd, int64_t _bufferLen, int flags, const sockaddr *to);

    /**
     * @brief 소멸자
     */
    ~RTCPPacket() = default;

private:
#if __BYTE_ORDER == __LITTLE_ENDIAN
    uint8_t rc : 5;        ///< 리포트 카운트
    uint8_t p : 1;         ///< 패딩 비트
    uint8_t version : 2;   ///< 버전 정보 (2)
#elif __BYTE_ORDER == __BIG_ENDIAN
    uint8_t version : 2;   ///< 버전 정보 (2)
    uint8_t p : 1;         ///< 패딩 비트
    uint8_t rc : 5;        ///< 리포트 카운트
#else
#error "Please fix <bits/endian.h>"
#endif
    uint8_t pt;              ///< 패킷 타입 (200: Sender Report)
    uint16_t length;         ///< 패킷 길이
    uint32_t ssrc;           ///< 동기화 소스 식별자
    uint32_t ntpTimestampMsw; ///< NTP 타임스탬프 MSW
    uint32_t ntpTimestampLsw; ///< NTP 타임스탬프 LSW
    uint32_t rtpTimestamp;    ///< RTP 타임스탬프
    uint32_t senderPacketCount; ///< 전송된 패킷 수
    uint32_t senderOctetCount;  ///< 전송된 총 바이트 수
};

#pragma pack()
