#ifndef RTSP_PROTOS_H
#define RTSP_PROTOS_H

#include <cstdint>
#include <cstddef>

enum ssrc
{
    PROTO_OPUS = 111,
    PROTO_H264 = 96,
};

class Protos
{
public:
    Protos();

    struct SenderReport {
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
    } __attribute__((packed));

    void CreateSR(SenderReport *sr, unsigned int timestamp, unsigned int packetCount, unsigned int octetCount, ssrc payloadType);
};

#endif //RTSP_PROTOS_H
