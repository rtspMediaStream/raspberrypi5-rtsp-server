/**
* @file RTPPacket.cpp
* @brief RTPPacket 클래스의 구현부
* @details RTPPacket 클래스의 멤버 함수를 구현한 소스 파일
* 
* Copyright (c) 2024 rtspMediaStream
* This project is licensed under the MIT License - see the LICENSE file for details
*/

#include <RTPPacket.hpp>

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <arpa/inet.h>
#include <netinet/in.h>

/**
* @details 
*   멤버 초기화 리스트를 사용하여 RTP 헤더를 초기화하고,
*   RTP_Payload는 0으로 초기화된다 (클래스 선언에서 {0}으로 초기화)
*/
RTPPacket::RTPPacket(const RTPHeader &rtpHeader) : header(rtpHeader) {}

/**
* @details 
*   - 페이로드 버퍼의 특정 위치(bias)부터 데이터를 복사
*   - std::min을 사용하여 버퍼 오버플로우 방지
*   - 버퍼 크기는 FU_SIZE + MAX_RTP_DATA_SIZE로 제한
*/
void RTPPacket::load_data(const uint8_t *data, const int64_t dataSize, const int64_t bias)
{
    memcpy(this->RTP_Payload + bias, data, std::min(dataSize, static_cast<int64_t>(sizeof(this->RTP_Payload) - bias)));
}

/**
* @details 
*   - sendto() 함수를 사용하여 UDP로 패킷 전송
*   - 전송 후 시퀀스 번호 자동 증가
*   - sockaddr 구조체는 수신자의 IP 주소와 포트 정보를 포함
*/
int64_t RTPPacket::rtp_sendto(int sockfd, const int64_t _bufferLen, const int flags, const sockaddr *to)
{
    auto sentBytes = sendto(sockfd, this, _bufferLen, flags, to, sizeof(sockaddr));
    header.set_seq(header.get_seq() + 1);
    return sentBytes;
}
