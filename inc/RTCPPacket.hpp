/**
 * @file RTCPPacket.hpp
 * @brief rtcp 패킷 정의
 * @details 이 파일은 h264 페이로드의 rtcp 패킷를 정의한 파일입니다.
 * @author InHak Jeon
 * @date 2024/12/31
 * @version 0.0.1
 */

#pragma once

#include <cstddef>
#include <cstdint>

#include <arpa/inet.h>
#include "RTSPServer.h"

#pragma pack(1)
class RTCPPacket
{
public:
    RTCPPacket(const unsigned int timestamp, unsigned int packetCount, unsigned int octetCount, Protocol payloadType);

    ~RTCPPacket() = default;

private:
#if __BYTE_ORDER == __LITTLE_ENDIAN
    uint8_t rc : 5;
    uint8_t p : 1;
    uint8_t version : 2;
#elif __BYTE_ORDER == __BIG_ENDIAN
    uint8_t version : 2;
    uint8_t p : 1;
    uint8_t rc : 5;

#else
#error "Please fix <bits/endian.h>"
#endif
    uint8_t pt;
    uint16_t length;
    uint32_t ssrc;
    uint32_t ntpTimestampMsw;
    uint32_t ntpTimestampLsw;
    uint32_t rtpTimestamp;
    uint32_t senderPacketCount;
    uint32_t senderOctetCount;
};

#pragma pack()
