/**
 * @file main.cpp
 * @brief RTSP 오디오 스트리밍 서버의 메인 파일
 * @details Opus 코덱을 사용한 오디오 스트리밍 서버의 구현부로,
 *          PCM 오디오를 캡처하고 Opus로 인코딩하여 RTSP로 스트리밍합니다.
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
#include "OpusEncoder.h"
#include "AudioCapture.h"

#include <thread>
#include "H264Encoder.h"
#include "DataCapture.h"
#include "Global.h"


/**
 * @brief Opus 오디오 스트리밍을 처리하는 함수
 * @details 별도의 스레드에서 실행되며 다음과 같은 작업을 수행합니다:
 *          1. PCM 오디오 데이터 캡처
 *          2. Opus 인코딩
 *          3. 인코딩된 프레임을 DataCapture 버퍼에 저장
 * 
 * 프로세스 흐름:
 * - PCM 버퍼 할당 (OPUS_FRAME_SIZE * OPUS_CHANNELS)
 * - OpusEncoder 및 AudioCapture 인스턴스 생성
 * - 무한 루프로 오디오 캡처 및 인코딩
 * - 인코딩된 프레임을 DataCapture에 푸시
 * 
 * @note 이 함수는 detached 스레드로 실행되어 백그라운드에서 동작합니다.
 */
void LoadOpus()
{
    std::thread([]()->void {
                     short pcmBuffer[OPUS_FRAME_SIZE * OPUS_CHANNELS];
                     OpusEncoder opusEncoder;
                     DataCaptureFrame newFrame;
                     newFrame.timestamp = (unsigned int)GetRanNum(16);
                     AudioCapture audioCapture;
                     while(1){
                        newFrame.dataPtr = new unsigned char[MAX_PACKET_SIZE];
                         
                        int rc = audioCapture.read(pcmBuffer, OPUS_FRAME_SIZE);
                        if (rc != OPUS_FRAME_SIZE)
                        {
                            std::cout << "occur audio packet skip." << std::endl;
                            continue;
                        }
                       
                        int bufferSize = opusEncoder.encode(pcmBuffer, OPUS_FRAME_SIZE, newFrame.dataPtr);
                        newFrame.size = bufferSize;
                        newFrame.timestamp += OPUS_FRAME_SIZE;
                        if (bufferSize <= 0)
                        {
                            std::cerr << "Opus encoding error: " << bufferSize << std::endl;
                            delete[] newFrame.dataPtr;
                            continue;
                        }

                        AudioCapture::getInstance().pushFrame(newFrame);
                     }
                     return ;
                 } ).detach();
}

/**
 * @brief 메인 함수
 * @param argc 명령행 인자 개수
 * @param argv 명령행 인자 배열
 * @return int 프로그램 종료 코드 (0: 정상 종료)
 * 
 * @details RTSP 서버를 초기화하고 시작하는 주요 단계:
 * 1. RTSP 서버 프로토콜을 Opus로 설정
 * 2. 초기화 이벤트 핸들러로 LoadOpus 함수 등록
 * 3. 서버 스레드 시작
 * 4. 무한 루프로 서버 유지
 */
int main(int argc, char *argv[])
{
    RTSPServer::getInstance().setProtocol(Protocol::PROTO_OPUS);
    RTSPServer::getInstance().onInitEvent = LoadOpus;
    RTSPServer::getInstance().startServerThread();

    while (1)
    {
        sleep(1);
    }
    
        
    return 0;
}