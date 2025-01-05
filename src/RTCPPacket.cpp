/**
 * @file RTCPPacket.cpp
 * @brief RTCPPacket 클래스의 구현부
 * @details RTCPPacket 클래스의 멤버 함수를 구현한 소스 파일
 * 
 * Copyright (c) 2024 rtspMediaStream
 * This project is licensed under the MIT License - see the LICENSE file for details
 */

#include "RTCPPacket.hpp"
#include "Global.h"

/**
 * @details 
 *   - RTCP 버전 2로 설정
 *   - 패킷 타입을 200(SR)으로 설정
 *   - 패킷 길이를 6으로 설정
 *   - SSRC를 페이로드 타입으로 설정
 *   - NTP 타임스탬프 생성 및 설정
 *   - 네트워크 바이트로 변환하여 설정
 *   - RTP 타임스탬프 설정
 *   - 전송된 패킷 수 설정 
 *   - 전송된 바이트 수 설정 (옥텟 수)
 */

RTCPPacket::RTCPPacket(const unsigned int timestamp, unsigned int packetCount, unsigned int octetCount, Protocol payloadType)
{
    version = 2;
    p = 0;
    rc = 0;
    pt = 200;
    length = htons(6);
    ssrc = htonl(payloadType);

    auto time = GetTime();
    ntpTimestampMsw = htonl((uint32_t)(time >> 32));
    ntpTimestampLsw = htonl((uint32_t)(time & 0xFFFFFFFF));
    rtpTimestamp = htonl(timestamp);
    senderPacketCount = htonl(packetCount);
    senderOctetCount = htonl(octetCount);
}

/**
 * @details RTCP 패킷을 UDP 소켓을 통해 지정된 주소로 전송하는 메서드
 */
int64_t RTCPPacket::rtcp_sendto(int sockfd, int64_t _bufferLen, int flags, const sockaddr *to)
{
    auto sentBytes = sendto(sockfd, this, _bufferLen, flags, to, sizeof(sockaddr));
    return sentBytes;
}
