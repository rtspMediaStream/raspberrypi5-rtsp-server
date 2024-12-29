#pragma once

#include <cstddef>
#include <cstdint>

#include <arpa/inet.h>

#include <RTPHeader.hpp>

class RTPPacket
{
private:
    RTPHeader header;
    uint8_t RTP_Payload[FU_SIZE + MAX_RTP_DATA_SIZE]{0};

public:

    explicit RTPPacket(const RTPHeader &rtpHeader);

    RTPPacket(const RTPPacket &) = default;
    ~RTPPacket() = default;

    void load_data(const uint8_t *data, int64_t dataSize, int64_t bias = 0);
    int64_t rtp_sendto(int sockfd, int64_t _bufferLen, int flags, const sockaddr *to);

    RTPHeader& get_header() { return this->header; }
    uint8_t *get_payload() { return this->RTP_Payload; }
};
#pragma pack()