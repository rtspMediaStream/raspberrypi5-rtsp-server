/**
 * @file RTPPacket.hpp
 * @brief RTP 패킷 생성 및 전송 클래스 헤더
 * @details RTP(Real-time Transport Protocol) 패킷 처리를 위한 클래스로,
*          패킷의 생성, 데이터 로드, UDP 전송을 담당
*          - RTP 헤더와 페이로드 관리
*          - 데이터 페이로드 설정
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

#include <RTPHeader.hpp>

/**
 * @class RTPPacket
 * @brief RTP 패킷을 생성하고 전송하는 클래스
 * @details RTP Packet = Header + Payload 로 구성된 패킷을 생성하고 전송하는 기능을 제공한다.
 * @see RTPHeader
 */
class RTPPacket
{
private:
    RTPHeader header;
    uint8_t RTP_Payload[FU_SIZE + MAX_RTP_DATA_SIZE]{0}; ///< RTP 페이로드 버퍼

public:
    /**
     * @brief RTPHeader를 가지는 RTPPacket 생성자
     * @param rtpHeader 초기화할 RTP 헤더 객체
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
    * @brief 페이로드 데이터를 설정하는 메서드
    * @param data 복사할 데이터의 포인터
    * @param dataSize 복사할 데이터의 크기
    * @param bias 페이로드 버퍼 내 오프셋 (기본값: 0)
    * @details 지정된 오프셋 위치부터 데이터를 페이로드 버퍼에 복사.
    *          버퍼 오버플로우 방지를 위해 최대 크기 제한
    */
    void load_data(const uint8_t *data, int64_t dataSize, int64_t bias = 0);

   /**
    * @brief UDP 소켓을 통해 RTP 패킷을 전송하는 메서드
    * @param sockfd UDP 소켓 디스크립터
    * @param _bufferLen 전송할 버퍼의 크기
    * @param flags 전송 옵션 플래그
    * @param to 수신자 주소 정보
    * @return int64_t 전송된 바이트 수
    * @details 패킷 전송 후 시퀀스 번호를 자동으로 증가시킴
    */
    int64_t rtp_sendto(int sockfd, int64_t _bufferLen, int flags, const sockaddr *to);

    /**
     * @brief RTP 헤더 참조를 반환하는 메서드
     * @details RTPPakcet에 설정된 RTPHeader의 참조를 가져온다.
     * @return RTPHeader& RTP 헤더 객체에 대한 참조
     */
    RTPHeader& get_header() { return this->header; }

    /**
     * @brief 페이로드 버퍼 포인터를 반환하는 메서드
     * @details RTPPakcet에 설정된 RTP Payload (=data)의 주소를 가져온다.
     * @return uint8_t* 페이로드 버퍼의 시작 주소
     */
    uint8_t *get_payload() { return this->RTP_Payload; }
};
#pragma pack()