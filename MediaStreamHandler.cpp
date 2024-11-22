#include "Protos.h"
#include "utils.h"
#include "TCPHandler.h"
#include "UDPHandler.h"
#include "MediaStreamHandler.h"
#include "AudioCapture.h"

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

MediaStreamHandler::MediaStreamHandler(): streamState(MediaStreamState::eMediaStream_Init){}

void MediaStreamHandler::HandleMediaStream() {
    Protos protos(utils::GetRanNum(32));

    size_t payloadSize = 160;  // 20ms당 160 샘플
    int frames = payloadSize;  // G.711은 8kHz에서 20ms당 160 샘플
    unsigned int sampleRate = 8000 ; // G.711의 샘플링 레이트
    int chnnels = 2; //mono

    short pcmBuffer[frames * chnnels];
    unsigned char payloadBuffer[payloadSize];

    unsigned int octetCount = 0;
    unsigned int packetCount = 0;
    unsigned short seqNum = (unsigned short)utils::GetRanNum(16);
    unsigned int timestamp = (unsigned int)utils::GetRanNum(16);

    Protos::SenderReport sr;
    Protos::RTPHeader rtpHeader;

    AudioCapture audioCapture(sampleRate);

    while (true) {
        if(streamState == MediaStreamState::eMediaStream_Pause) {
            std::unique_lock<std::mutex> lck(streamMutex);
            condition.wait(lck);
        }
        else if (streamState == MediaStreamState::eMediaStream_Teardown) {
            break;
        }
        else if(streamState == MediaStreamState::eMediaStream_Play) {
            int rc = audioCapture.read(pcmBuffer, frames);
            if (rc != frames) {
                continue;
            }

            //pcm ulaw encoding
            for (int i = 0; i < frames; i++)
                payloadBuffer[i] = pcm_to_ulaw(pcmBuffer[i]);

            //make RTP Packet.
            unsigned char rtpPacket[sizeof(Protos::RTPHeader) + payloadSize] = {0,};
            protos.CreateRTPHeader(&rtpHeader, seqNum, timestamp);
            memcpy(rtpPacket, &rtpHeader, sizeof(rtpHeader));
            memcpy(rtpPacket + sizeof(rtpHeader), payloadBuffer, payloadSize);

            std::cout << "RTP " << packetCount << " sent" << std::endl;

            udpHandler->SendRTPPacket(rtpPacket, sizeof(rtpPacket));

            seqNum++;
            timestamp += payloadSize;
            packetCount++;
            octetCount += payloadSize;

            if (packetCount % 100 == 0)
            {
                std::cout << "RTCP sent" << std::endl;
                protos.CreateSR(&sr, timestamp, packetCount, octetCount);
                udpHandler->SendSenderReport(&sr, sizeof(sr));
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(20));
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