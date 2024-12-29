#include "Protos.h"
#include "Global.h"

#include <arpa/inet.h>

Protos::Protos() {}


void Protos::CreateSR(SenderReport *sr, unsigned int timestamp, unsigned int packetCount, unsigned int octetCount, Protocol payloadType) {
    sr->version = 2;
    sr->p = 0;
    sr->rc = 0;
    sr->pt = 200;
    sr->length = htons(6);
    sr->ssrc = htonl(payloadType);

    auto time = GetTime();
    sr->ntpTimestampMsw = htonl((uint32_t)(time >> 32));
    sr->ntpTimestampLsw = htonl((uint32_t)(time & 0xFFFFFFFF));
    sr->rtpTimestamp = htonl(timestamp);
    sr->senderPacketCount = htonl(packetCount);
    sr->senderOctetCount = htonl(octetCount);
}