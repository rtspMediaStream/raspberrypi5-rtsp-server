#include "RTCPPacket.hpp"
#include "Global.h"

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

int64_t RTCPPacket::rtcp_sendto(int sockfd, int64_t _bufferLen, int flags, const sockaddr *to)
{
    auto sentBytes = sendto(sockfd, this, _bufferLen, flags, to, sizeof(sockaddr));
    return sentBytes;
}
