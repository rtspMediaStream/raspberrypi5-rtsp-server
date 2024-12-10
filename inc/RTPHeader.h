// RTP, RTCP, 오디오 캡처 및 MP3 인코딩을 처리하는 C++ 클래스 기반 프로그램 (싱글톤 패턴 적용)

#include <iostream>
#include <cstdlib>
#include <cstring>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <time.h>
#include <stdint.h>
#include <alsa/asoundlib.h>
#include <opus/opus.h>

#define DEST_IP "127.0.0.1"
#define RTP_PORT 5004
#define RTCP_PORT 5005
#define PCM_FRAME_SIZE 1152
#define APPLICATION OPUS_APPLICATION_AUDIO
#define MAX_PACKET_SIZE 4000

class RTPHeader {
public:
    struct rtp_header {
#if __BYTE_ORDER == __LITTLE_ENDIAN
        uint8_t cc:4;
        uint8_t x:1;
        uint8_t p:1;
        uint8_t version:2;
        uint8_t pt:7;
        uint8_t m:1;
#elif __BYTE_ORDER == __BIG_ENDIAN
        uint8_t version:2;
        uint8_t p:1;
        uint8_t x:1;
        uint8_t cc:4;
        uint8_t m:1;
        uint8_t pt:7;
#else
#error "Please fix <bits/endian.h>"
#endif
        uint16_t seq_num;
        uint32_t timestamp;
        uint32_t ssrc;
    } __attribute__((packed));

    static void create(rtp_header &header, unsigned short seq_num, unsigned int timestamp, unsigned int ssrc) {
        header.version = 2;
        header.p = 0;
        header.x = 0;
        header.cc = 0;
        header.m = 0;
        header.pt = 111;  
        header.seq_num = htons(seq_num);
        header.timestamp = htonl(timestamp);
        header.ssrc = htonl(ssrc);
    }
};