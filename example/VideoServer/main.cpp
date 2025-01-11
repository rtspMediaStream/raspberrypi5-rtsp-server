/**
 * @file main.cpp
 * @brief RTSP 비디오 파일 스트리밍 서버의 메인 파일
 * @details H.264 포멧의 비디오 스트리밍 서버의 구현부.
 * 
 * @organization rtspMediaStream
 * @repository https://github.com/rtspMediaStream/raspberrypi5-rtsp-server
 * 
 * Copyright (c) 2024 rtspMediaStream
 * This project is licensed under the MIT License - see the LICENSE file for details
 */
#include "RTSPServer.h"
#include <unistd.h>
#include <iostream>

#include <thread>
#include "H264Encoder.h"
#include "DataCapture.h"

/**
 * @brief Video frame을 처리하는 함수
 * @details H.264 format의 비디오 파일에서 frame을 일겅와 DataCapture에 추가합니다.
 */
void LoadH264File()
{
    std::thread([]() -> void
                {
                std::cout << "thread start"<<std::endl;
            constexpr int64_t target_frame_duration_us = 1000000 / 30; // 30 fps -> 33,333 microseconds per frame
            H264Encoder* h264_file = new H264Encoder("../dragon.h264");

            DataCaptureFrame frame;
            while (true) {
                auto frame_start_time = std::chrono::high_resolution_clock::now();

                // Get the next frame
                std::pair<const uint8_t *, int64_t> cur_frame = h264_file->get_next_frame();
                const uint8_t * framePtr = cur_frame.first;
                int64_t frameSize = cur_frame.second;
                if (framePtr == nullptr) {
                    return;
                }

                // split nalu start code 3 or 4 byte
                const int64_t naluStartLen = H264Encoder::is_start_code(framePtr, frameSize, 4) ? 4 : 3;

                frame.dataPtr = (unsigned char *)framePtr + naluStartLen;
                frame.size = frameSize - naluStartLen;
                frame.timestamp += 3000;

                // Process the frame
                DataCapture::getInstance().pushFrame(frame);

                // Calculate elapsed time
                auto frame_end_time = std::chrono::high_resolution_clock::now();
                auto elapsed_time_us = std::chrono::duration_cast<std::chrono::microseconds>(frame_end_time - frame_start_time).count();
                // Calculate remaining time to wait
                int64_t remaining_time_us = target_frame_duration_us - elapsed_time_us;
                if (remaining_time_us > 0) {
                    std::this_thread::sleep_for(std::chrono::microseconds(remaining_time_us));
                }
        } })
        .detach();
}

/**
 * @brief 메인 함수
 * @param argc 명령행 인자 개수
 * @param argv 명령행 인자 배열
 * @return int 프로그램 종료 코드 (0: 정상 종료)
 * 
 * @details RTSP 서버를 초기화하고 시작하는 주요 단계:
 * 1. RTSP 서버 프로토콜을 H264로 설정
 * 2. 초기화 이벤트 핸들러로 LoadH264File 함수 등록
 * 3. 서버 스레드 시작
 * 4. 무한 루프로 서버 유지
 */
int main(int argc, char *argv[])
{
    
    RTSPServer::getInstance().setProtocol(Protocol::PROTO_H264);
    RTSPServer::getInstance().onInitEvent = LoadH264File;
    RTSPServer::getInstance().startServerThread();

    while (1)
    {
        sleep(1);
    }
    
        
    return 0;
}