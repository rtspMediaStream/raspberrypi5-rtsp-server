#include "Protos.h"
#include "utils.h"
#include "TCPHandler.h"
#include "UDPHandler.h"
#include "MediaStreamHandler.h"
#include "AudioCapture.h"
#include "OpusEncoder.h"
#include "H264Encoder.h"
#include "global.h"

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
#define PCM_FRAME_SIZE 1152

constexpr int64_t IP_V4_HEADER_SIZE = 20;
constexpr int64_t UDP_HEADER_SIZE = 8;
constexpr int64_t RTP_HEADER_SIZE = 12;
constexpr int64_t RTP_VERSION = 2;
constexpr int64_t RTP_PAYLOAD_TYPE_H264 = 96;
constexpr int64_t FU_SIZE = 2;  // FU-A 헤더 크기 (FU Indicator + FU Header)

constexpr int64_t MAX_UDP_PACKET_SIZE = 65535;
constexpr int64_t MAX_RTP_DATA_SIZE = MAX_UDP_PACKET_SIZE - IP_V4_HEADER_SIZE
                                        - UDP_HEADER_SIZE - RTP_HEADER_SIZE - FU_SIZE;
constexpr int64_t MAX_RTP_PACKET_LEN = MAX_RTP_DATA_SIZE + RTP_HEADER_SIZE + FU_SIZE;


MediaStreamHandler::MediaStreamHandler(): streamState(MediaStreamState::eMediaStream_Init){}

void MediaStreamHandler::SendFragmentedRTPPackets(unsigned char* payload, size_t payloadSize, Protos::RTPHeader& rtpHeader, unsigned short &seqNum) {
    unsigned char nalHeader = payload[0]; // NAL 헤더 (첫 바이트)
    unsigned char rtpPacket[MAX_UDP_PACKET_SIZE] = {0};

    if (payloadSize <= MAX_RTP_DATA_SIZE) {
        // 패킷 크기가 MTU 이하인 경우, 단일 RTP 패킷 전송
        //rtpHeader.m = 1; // 단일 RTP 패킷이므로 마커 비트 활성화
        memcpy(rtpPacket, &rtpHeader, sizeof(Protos::RTPHeader)); // RTP 헤더 복사
        memcpy(rtpPacket + sizeof(Protos::RTPHeader), payload, payloadSize); // NAL 데이터 복사

        std::cout<<"test02"<<std::endl;
        udpHandler->SendRTPPacket(rtpPacket, sizeof(Protos::RTPHeader) + payloadSize);

        // 마커 비트 설정
        seqNum++;
        rtpHeader.seqNum = seqNum;
        return;
    }

    // 패킷 크기가 MTU를 초과하는 경우, FU-A로 분할
    //unsigned char fuIndicator = (nalHeader & 0xE0) | 28; // FU Indicator (F | NRI | Type)


    const int64_t packetNum = payloadSize / MAX_RTP_DATA_SIZE;
    const int64_t remainPacketSize = payloadSize % MAX_RTP_DATA_SIZE;
    int64_t pos = 1;    // NAL 헤더(첫 바이트)는 별도 처리

    for (int i = 0; i < packetNum; i++) {
        rtpPacket[sizeof(Protos::RTPHeader) + 0] = (nalHeader & NALU_F_NRI_MASK) | SET_FU_A_MASK;
        rtpPacket[sizeof(Protos::RTPHeader) + 1] = nalHeader & NALU_TYPE_MASK;
        // FU Header 생성
        if(i == 0) {    //처음 조각
            rtpPacket[sizeof(Protos::RTPHeader) + 1] |= FU_S_MASK;
        }else if(i == packetNum-1 && remainPacketSize == 0) {    //마지막 조각
            rtpPacket[sizeof(Protos::RTPHeader) + 1] |= FU_E_MASK;
        }

        // RTP 패킷 생성
        memcpy(rtpPacket, &rtpHeader, sizeof(Protos::RTPHeader));                                          // RTP Header 추가
        memcpy(rtpPacket + sizeof(Protos::RTPHeader) + FU_SIZE, &payload[pos], MAX_RTP_DATA_SIZE); // 분할된 데이터 복사
        // RTP 패킷 전송
        std::cout << "test01" << std::endl;
        udpHandler->SendRTPPacket(rtpPacket, MAX_RTP_PACKET_LEN);

        // 시퀀스 넘버 증가 및 오프셋 업데이트
        seqNum++;
        rtpHeader.seqNum = seqNum;
        pos += MAX_RTP_DATA_SIZE;
    }
    if(remainPacketSize > 0) {
        rtpPacket[sizeof(Protos::RTPHeader) + 0] = (nalHeader & NALU_F_NRI_MASK) | SET_FU_A_MASK;
        rtpPacket[sizeof(Protos::RTPHeader) + 1] = (nalHeader & NALU_TYPE_MASK) | FU_E_MASK;

        // RTP 패킷 생성
        memcpy(rtpPacket, &rtpHeader, sizeof(Protos::RTPHeader));                                          // RTP Header 추가
        memcpy(rtpPacket + sizeof(Protos::RTPHeader) + FU_SIZE, payload + pos, remainPacketSize); // 분할된 데이터 복사
        // RTP 패킷 전송
        std::cout << "test01" << std::endl;
        //udpHandler->SendRTPPacket(rtpPacket, sizeof(Protos::RTPHeader) + sizeof(FU_SIZE) + remainPacketSize);

        seqNum++;
        rtpHeader.seqNum = seqNum;
    }
}

void MediaStreamHandler::HandleMediaStream() {
    Protos protos;

    short pcmBuffer[OPUS_FRAME_SIZE * OPUS_CHANNELS];
    unsigned char encodedBuffer[MAX_PACKET_SIZE];

    unsigned int octetCount = 0;
    unsigned int packetCount = 0;
    unsigned short seqNum = (unsigned short)utils::GetRanNum(16);
    unsigned int timestamp = (unsigned int)utils::GetRanNum(16);

    Protos::SenderReport sr;
    Protos::RTPHeader rtpHeader;

    
    AudioCapture *audioCapture = nullptr;
    OpusEncoder *opusEncoder = nullptr;
    H264Encoder *h264_file = nullptr;

    if(ServerStream::getInstance().type == Audio) {
        std::cout<< "audio" << std::endl;
        audioCapture = new AudioCapture(OPUS_SAMPLE_RATE);
        opusEncoder = new OpusEncoder();
    }else if(ServerStream::getInstance().type == Video){
        std::cout<< "video file open : " << g_inputFile.c_str() << std::endl;
        h264_file = new H264Encoder(g_inputFile.c_str());
    }

    while (true) {
        if(streamState == MediaStreamState::eMediaStream_Pause) {
            std::unique_lock<std::mutex> lck(streamMutex);
            condition.wait(lck);
        }
        else if (streamState == MediaStreamState::eMediaStream_Teardown) {
            break;
        }
        else if(streamState == MediaStreamState::eMediaStream_Play) {
            if(ServerStream::getInstance().type == Audio) {
                int rc = audioCapture->read(pcmBuffer, OPUS_FRAME_SIZE);
                if (rc != OPUS_FRAME_SIZE)
                {
                    continue;
                }

                int encoded_bytes = opusEncoder->encode(pcmBuffer, OPUS_FRAME_SIZE, encodedBuffer);

                // make RTP Packet.
                unsigned char rtpPacket[sizeof(Protos::RTPHeader) + encoded_bytes] = { 0, };
                protos.CreateRTPHeader(&rtpHeader, seqNum, timestamp, PROTO_OPUS);
                memcpy(rtpPacket, &rtpHeader, sizeof(rtpHeader));
                memcpy(rtpPacket + sizeof(rtpHeader), encodedBuffer, encoded_bytes);
                std::cout<<"test03"<<std::endl;
                udpHandler->SendRTPPacket(rtpPacket, sizeof(rtpPacket));

                seqNum++;
                timestamp += PCM_FRAME_SIZE;
                packetCount++;
                octetCount += encoded_bytes;

                if (packetCount % 100 == 0)
                {
                    std::cout << "RTCP sent" << std::endl;
                    protos.CreateSR(&sr, timestamp, packetCount, octetCount, PROTO_OPUS);
                    udpHandler->SendSenderReport(&sr, sizeof(sr));
                }
            }
            if (ServerStream::getInstance().type == Video) {
                auto cur_frame = h264_file->get_next_frame();
                const auto ptr_cur_frame = cur_frame.first;
                const auto cur_frame_size = cur_frame.second;

                if (ptr_cur_frame == nullptr || cur_frame_size <= 0)
                {
                    std::cerr << "No more frames or invalid frame data." << std::endl;
                    break;
                }


                // RTP 헤더 생성
                protos.CreateRTPHeader(&rtpHeader, seqNum, timestamp, PROTO_H264);

                // RTP 패킷 전송 (FU-A 분할 포함)
                const int64_t start_code_len = H264Encoder::is_start_code(ptr_cur_frame, cur_frame_size, 4) ? 4 : 3;
                SendFragmentedRTPPackets((unsigned char*)ptr_cur_frame+ start_code_len, cur_frame_size- start_code_len, rtpHeader, seqNum);

                // 타임스탬프 업데이트
                timestamp += 3000.0; //uint32_t(90000 / 30);

                // 주기적으로 RTCP Sender Report 전송
                packetCount++;
                octetCount += cur_frame_size;

                // if (packetCount % 100 == 0)
                // {
                //     std::cout << "RTCP sent" << std::endl;
                //     protos.CreateSR(&sr, timestamp, packetCount, octetCount, PROTO_H264);
                //     udpHandler->SendSenderReport(&sr, sizeof(sr));
                // }
                
                const auto sleepPeriod = uint32_t(1000 * 1000 / 30.0);
                usleep(sleepPeriod);
            }
        }

    }
}

#define BIAS 0x84
#define CLIP 32635
uint8_t MediaStreamHandler::pcm_to_ulaw(int16_t pcm_val) {
    int16_t mask;
    int16_t seg;
    uint8_t uval;

    // Handle negative values
    if (pcm_val < 0) {
        pcm_val = BIAS - pcm_val;
        mask = 0x7F;
    } else {
        pcm_val += BIAS;
        mask = 0xFF;
    }

    // Clip the value
    if (pcm_val > CLIP) pcm_val = CLIP;

    // Convert PCM value to segment and μ-law value
    seg = 8;
    for (int16_t value = pcm_val; value >= (1 << (seg + 3)); seg++) {}

    uval = (seg << 4) | ((pcm_val >> (seg + 3)) & 0x0F);
    return ~uval & mask;
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