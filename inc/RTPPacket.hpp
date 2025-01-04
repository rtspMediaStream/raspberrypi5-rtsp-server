/**
 * @file RTPPacket.h
 * @brief RTP 패킷을 생성 및 전송
 * @details RTP 패킷 생성 및 전송 클래스
 *          - RTP 패킷 생성
 *          - RTP Header 생성
 *          - RTP Payload 설정
 *          - RTP 패킷 전송
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

#include <RTPHeader.hpp>

/**
 * @class RTPPacket
 * @brief RTP 패킷을 생성하고 전송하는 클래스
 * @details RTP Packet = Header + Payload 로 구성된 패킷을 생성하고 전송하는 기능을 제공합니다.
 */
class RTPPacket
{
private:
    RTPHeader header;
    uint8_t RTP_Payload[FU_SIZE + MAX_RTP_DATA_SIZE]{0};

public:
    /**
     * @brief RTPHeader를 가지는 RTPPacket 생성자
     */
    explicit RTPPacket(const RTPHeader &rtpHeader);

    /**
     * @brief 기본 복사생성자
     */
    RTPPacket(const RTPPacket &) = default;

    /**
     * @brief 기본 소멸자
     */
    ~RTPPacket() = default;

    /**
     * @brief RTPPacket 에 payload(data) 를 설정합니다.
     * @details RTP paylaod 위치에 *data 부터 dataSize byte 를 복사합니다.
     */
    void load_data(const uint8_t *data, int64_t dataSize, int64_t bias = 0);

    /**
     * @brief RTPPacket 을 전송합니다.
     * @details sock file descriptor로 패킷을 전송합니다.
     * @return 패킷 전송 결과
     */
    int64_t rtp_sendto(int sockfd, int64_t _bufferLen, int flags, const sockaddr *to);

    /**
     * @brief RTP Header를 가져옵니다.
     * @details RTPPakcet에 설정된 RTPHeader의 참조를 가져옵니다.
     * @return RTPHeader&
     */
    RTPHeader& get_header() { return this->header; }

    /**
     * @brief RTP Payload를 가져옵니다.
     * @details RTPPakcet에 설정된 RTP Payload (=data)의 주소를 가져옵니다.
     * @return uint8_int
     */
    uint8_t *get_payload() { return this->RTP_Payload; }
};
#pragma pack()