/**
 * @file MediaStreamHandler.cpp
 * @brief MediaStreamHandler 클래스의 구현부
 * @details MediaStreamHandler 클래스의 멤버 함수를 구현한 소스 파일
 * 
 * Copyright (c) 2024 rtspMediaStream
 * This project is licensed under the MIT License - see the LICENSE file for details
 */

#include "Protos.h"
#include "Global.h"
#include "TCPHandler.h"
#include "UDPHandler.h"
#include "MediaStreamHandler.h"
#include "DataCapture.h"
#include "OpusEncoder.h"
#include "H264Encoder.h"
#include "RTPHeader.hpp"
#include "RTPPacket.hpp"

#include <iostream>
#include <cstdint>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <chrono>
#include <thread>
#include <mutex>
#include <utility>
#include <random>
#include <algorithm>

/**
 * @brief MediaStreamHandler 생성자
 * @details 스트림 상태를 초기화 상태로 설정
 */
MediaStreamHandler::MediaStreamHandler(): streamState(MediaStreamState::eMediaStream_Init){}

/**
 * @brief RTP 패킷을 프래그먼트화하여 전송하는 메서드
 * @param payload 전송할 페이로드 데이터
 * @param payloadSize 전송할 페이로드 데이터 크기
 * @param rtpPacket RTP 패킷 객체
 * @param timeStamp RTP 패킷의 타임스탬프
 * @details
 *   - 페이로드 크기에 따라 단일 패킷 또는 분할 패킷으로 전송
 *   - FU-A 형식으로 NAL 단위 분할 처리
 *   - RTP 헤더 마커 비트 설정
 */
void MediaStreamHandler::SendFragmentedRTPPackets(unsigned char* payload, size_t payloadSize, RTPPacket& rtpPacket, const uint32_t timeStamp) {
    unsigned char nalHeader = payload[0]; // NAL 헤더 (첫 바이트)

    if (payloadSize <= MAX_RTP_DATA_SIZE) {
        // 마커 비트 설정
        rtpPacket.get_header().set_marker(1); // 단일 RTP 패킷이므로 마커 비트 활성화

        // 패킷 크기가 MTU 이하인 경우, 단일 RTP 패킷 전송
        memcpy(rtpPacket.get_payload(), payload, payloadSize); // NAL 데이터 복사

        rtpPacket.get_header().set_timestamp(timeStamp);
        rtpPacket.rtp_sendto(udpHandler->GetRTPSocket(), RTP_HEADER_SIZE + payloadSize, 0, (struct sockaddr *)(&udpHandler->GetRTPAddr()));
        return;
    }

    const int64_t packetNum = payloadSize / MAX_RTP_DATA_SIZE;
    const int64_t remainPacketSize = payloadSize % MAX_RTP_DATA_SIZE;
    int64_t pos = 1;    // NAL 헤더(첫 바이트)는 별도 처리

    // 패킷 크기가 MTU를 초과하는 경우, FU-A로 분할
    for (int i = 0; i < packetNum; i++) {
        rtpPacket.get_payload()[0] = (nalHeader & NALU_F_NRI_MASK) | SET_FU_A_MASK;
        rtpPacket.get_payload()[1] = nalHeader & NALU_TYPE_MASK;
        rtpPacket.get_header().set_marker(0);

        if (i == 0)
        {
            // 첫 번째 조각: FU-A Start
            rtpPacket.get_payload()[1] = (nalHeader & NALU_TYPE_MASK) | FU_S_MASK;
        }
        else if (i == packetNum - 1 && remainPacketSize == 0)
        {
            // 마지막 조각: FU-A End
            rtpPacket.get_payload()[1] = (nalHeader & NALU_TYPE_MASK) | FU_E_MASK;
        }
        else
        {
            // 중간 조각: FU-A Middle
            rtpPacket.get_payload()[1] = (nalHeader & NALU_TYPE_MASK);
        }

        // RTP 패킷 생성
        memcpy(rtpPacket.get_payload() + FU_SIZE, &payload[pos], MAX_RTP_DATA_SIZE); // 분할된 데이터 복사
        rtpPacket.rtp_sendto(udpHandler->GetRTPSocket(), MAX_RTP_PACKET_LEN, 0, (struct sockaddr*)(&udpHandler->GetRTPAddr()));

        pos += MAX_RTP_DATA_SIZE;
    }
    // 남은 데이터 처리
    if(remainPacketSize > 0) {
        rtpPacket.get_payload()[0] = (nalHeader & NALU_F_NRI_MASK) | SET_FU_A_MASK;
        rtpPacket.get_payload()[1]= (nalHeader & NALU_TYPE_MASK) | FU_E_MASK;
        
        rtpPacket.get_header().set_marker(1);
        // RTP 패킷 생성
        memcpy(rtpPacket.get_payload() + FU_SIZE, &payload[pos], remainPacketSize); // 분할된 데이터 복사
        rtpPacket.rtp_sendto(udpHandler->GetRTPSocket(), RTP_HEADER_SIZE + FU_SIZE + remainPacketSize, 0, (struct sockaddr *)(&udpHandler->GetRTPAddr()));
    }
}

/**
 * @brief 미디어 스트림을 처리하는 메인 메서드
 * @details
 *   - 스트림 상태에 따라 미디어 데이터 처리
 *   - DataCapture로부터 프레임 데이터 획득
 *   - RTP 패킷 생성 및 전송
 *   - RTCP Sender Report 주기적 전송
 */
void MediaStreamHandler::HandleMediaStream() {
    Protos protos;

    unsigned int octetCount = 0;
    unsigned int packetCount = 0;
    uint16_t seqNum = (uint16_t)GetRanNum(16);

    Protos::SenderReport sr;
    int ssrcNum = 0;

    // RTP 헤더 생성
    RTPHeader rtpHeader(0, 0, ssrcNum);
    rtpHeader.set_payloadType(RTSPServer::getInstance().getProtocol());
    rtpHeader.set_seq(seqNum);

    // RTP 패킷 생성
    RTPPacket rtpPack{rtpHeader};

    while (true) {
        if(streamState == MediaStreamState::eMediaStream_Play) {
            while (!DataCapture::getInstance().isEmptyBuffer())
            {
                DataCaptureFrame cur_frame = DataCapture::getInstance().popFrame();
                const auto frame_ptr = cur_frame.dataPtr;
                const auto frame_size = cur_frame.size;
                const auto timestamp = cur_frame.timestamp;
                if (frame_ptr == nullptr || frame_size <= 0)
                {
                    std::cout << "Not Ready\n";
                    continue;
                }

                if (packetCount % 100 == 0)
                {
                    rtpPack.get_header().set_marker(true);
                }
                else
                {
                    rtpPack.get_header().set_marker(false);
                }

                // split FU-A
                SendFragmentedRTPPackets((unsigned char *)frame_ptr, frame_size, rtpPack, timestamp);

                // 주기적으로 RTCP Sender Report 전송
                packetCount++;
                octetCount += frame_size;

                // if (packetCount % 100 == 0)
                // {
                //     std::cout << "RTCP sent" << std::endl;
                //     protos.CreateSR(&sr, timestamp, packetCount, octetCount, PROTO_H264);
                //     udpHandler->SendSenderReport(&sr, sizeof(sr));
                // }
            }
        }else if(streamState == MediaStreamState::eMediaStream_Pause) {
            std::unique_lock<std::mutex> lck(streamMutex);
            condition.wait(lck);
        }
        else if (streamState == MediaStreamState::eMediaStream_Teardown) {
            break;
        }
    }
}

/**
 * @brief 스트리밍 명령을 설정하는 메서드
 * @param cmd 설정할 명령 ("PLAY", "PAUSE", "TEARDOWN")
 * @details
 *   - 스트림 상태를 변경하고 조건 변수를 통해 스레드 제어
 *   - 뮤텍스를 사용하여 스레드 안전성 보장
 */
void MediaStreamHandler::SetCmd(const std::string& cmd) {
    std::lock_guard<std::mutex> lock(streamMutex);
    if (cmd == "PLAY") {
        streamState = MediaStreamState::eMediaStream_Play;
        condition.notify_all();
    } else if (cmd == "PAUSE") {
        streamState = MediaStreamState::eMediaStream_Pause;
    } else if (cmd == "TEARDOWN") {
        streamState = MediaStreamState::eMediaStream_Teardown;
    }
}