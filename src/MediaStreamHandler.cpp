#include "Protos.h"
#include "utils.h"
#include "TCPHandler.h"
#include "UDPHandler.h"
#include "MediaStreamHandler.h"
#include "DataCapture.h"
#include "OpusEncoder.h"
#include "H264Encoder.h"
#include "global.h"
#include "rtp_header.hpp"
#include "rtp_packet.hpp"
#include "RTPHeader.h"

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

MediaStreamHandler::MediaStreamHandler(): streamState(MediaStreamState::eMediaStream_Init){}

void MediaStreamHandler::SendFragmentedRTPPackets(unsigned char* payload, size_t payloadSize, RtpPacket& rtpPacket, const uint32_t timeStamp) {
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
    if(remainPacketSize > 0) {
        rtpPacket.get_payload()[0] = (nalHeader & NALU_F_NRI_MASK) | SET_FU_A_MASK;
        rtpPacket.get_payload()[1]= (nalHeader & NALU_TYPE_MASK) | FU_E_MASK;
        
        rtpPacket.get_header().set_marker(1);
        // RTP 패킷 생성
        memcpy(rtpPacket.get_payload() + FU_SIZE, &payload[pos], remainPacketSize); // 분할된 데이터 복사
        rtpPacket.rtp_sendto(udpHandler->GetRTPSocket(), RTP_HEADER_SIZE + FU_SIZE + remainPacketSize, 0, (struct sockaddr *)(&udpHandler->GetRTPAddr()));
    }
}

void MediaStreamHandler::HandleMediaStream() {
    Protos protos;

    unsigned int octetCount = 0;
    unsigned int packetCount = 0;
    uint16_t seqNum = (uint16_t)utils::GetRanNum(16);

    Protos::SenderReport sr;
    int ssrcNum = 0;

    // RTP 헤더 생성
    RtpHeader rtpHeader(0, 0, ssrcNum);
    rtpHeader.set_payloadType(RTSPServer::getInstance().getProtocol());
    rtpHeader.set_seq(seqNum);

    // RTP 패킷 생성
    RtpPacket rtpPack{rtpHeader};

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