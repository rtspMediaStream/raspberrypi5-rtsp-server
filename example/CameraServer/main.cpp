/**
 * @file main.cpp
 * @brief RTSP 카메라 스트리밍 서버의 메인 파일
 * @details FFmpeg Library를 이용한 카메라 스트리밍 서버의 구현부,
 *          FFmpeg으로 인코딩하여 RTSP로 스트리밍합니다.
 * 
 * @organization rtspMediaStream
 * @repository https://github.com/rtspMediaStream/raspberrypi5-rtsp-server
 * 
 * Copyright (c) 2024 rtspMediaStream
 * This project is licensed under the MIT License - see the LICENSE file for details
 */

#include "RTSPServer.h"
#include "ClientSession.h"
#include "RequestHandler.h"
#include "Global.h"
#include "DataCapture.h"

#include <string>
#include <thread>
#include <iostream>
#include <csignal>
#include <vector>
#include <fstream>
#include <iomanip>
#include <memory>
#include <sys/mman.h> // mmap, munmap
#include <opencv2/opencv.hpp>
#include "libcamera/libcamera.h"
#include "FFmpegEncoder.h"
/// camera check in raspberry pi
extern "C"
{
    #include "libavformat/avformat.h"
    #include "libavcodec/avcodec.h"
    #include "libavutil/avutil.h"
    #include "libavutil/opt.h"
    #include "libswscale/swscale.h"
}
using namespace libcamera;
using namespace std;
static std::shared_ptr<libcamera::Camera> camera;
FFmpegEncoder ffmpegEncoder("output.h264",640, 480, 30.0);

/**
 * @brief 카메라 frame을 처리하는 함수
 * @details 카메라 모듈의 frameBuffer에 접근해 yuv로 된 이미지를 가져와 mat형태로 변환하여 처리
 */
static void requestComplete(Request *request)
{
	if (request->status() == Request::RequestCancelled)
		return;

    const std::map<const Stream *, FrameBuffer *> &buffers = request->buffers();
    for (auto bufferPair : buffers) {
        FrameBuffer *buffer = bufferPair.second;
        const FrameMetadata &metadata = buffer->metadata();


        std::cout << " seq: " << std::setw(6) << std::setfill('0') << metadata.sequence << " bytesused: ";
        unsigned int nplane = 0;
        for (const FrameMetadata::Plane &plane : metadata.planes())
        {
            std::cout << plane.bytesused;
            
            if (++nplane < metadata.planes().size()) std::cout << "/";
        }
        std::cout << std::endl;
        
         // Plane 데이터 접근
        const FrameBuffer::Plane &plane = buffer->planes()[0];
        int fd = plane.fd.get(); // 파일 디스크립터 가져오기
        unsigned int offset = plane.offset;
        unsigned int length = plane.length;

        // mmap을 통해 메모리에 매핑
        void *mappedMemory = mmap(nullptr, length, PROT_READ, MAP_SHARED, fd, offset);
        if (mappedMemory == MAP_FAILED) {
            perror("mmap failed");
            continue;
        }

        int width = 640;  // 기본값: 640x480
        int height = 480;

        // XRGB8888 -> YUV420p 변환
        uint8_t *src = static_cast<uint8_t *>(mappedMemory);
        cv::Mat yuvImage(height + height / 2, width, CV_8UC1); // YUV420p 데이터를 하나의 Mat로 저장

        uint8_t *yPlane = yuvImage.data;                       // Y 채널 시작
        uint8_t *uPlane = yPlane + (height * width);           // U 채널 시작
        uint8_t *vPlane = uPlane + (height * width) / 4;       // V 채널 시작

        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                uint32_t pixel = reinterpret_cast<uint32_t *>(src)[y * width + x]; // XRGB8888 한 픽셀 읽기
                uint8_t r = (pixel >> 16) & 0xFF; // R 추출
                uint8_t g = (pixel >> 8) & 0xFF;  // G 추출
                uint8_t b = pixel & 0xFF;         // B 추출

                // YUV420p의 Y 계산
                yPlane[y * width + x] = static_cast<uint8_t>(0.299 * r + 0.587 * g + 0.114 * b);

                // U와 V는 2x2 블록마다 계산
                if (y % 2 == 0 && x % 2 == 0) {
                    int uvIndex = (y / 2) * (width / 2) + (x / 2);

                    // U와 V 계산 (다운샘플링)
                    uPlane[uvIndex] = static_cast<uint8_t>(-0.169 * r - 0.331 * g + 0.5 * b + 128);
                    vPlane[uvIndex] = static_cast<uint8_t>(0.5 * r - 0.419 * g - 0.081 * b + 128);
                }
            }
        }
        // yuvImage는 YUV420p 데이터를 포함하며, 각 채널은 Y, U, V 순서로 저장되어 있습니다.

        ffmpegEncoder.encode(yuvImage, 30);

        // 매핑 해제
        munmap(mappedMemory, length);
    }

    request->reuse(Request::ReuseBuffers);
}

/**
 * @brief Camera 모듈에서 이미지를 처리하는 Thread
 * @details 스레드실행시 처리 작업을 수행합니다:
 *          1. Raspberry Pi5 의 모듈 카메라에 연결
 *          2. 카메라 캡처시 동작할 함수(requestComplete) 설정
 *          3. Thread에서 캡처 처리
 */
void cameraThread()
{
    std::thread([]() -> int
                {
        //camera thread
        static std::shared_ptr<libcamera::Camera> camera;
        std::unique_ptr<CameraManager> cm = std::make_unique<CameraManager>();
        cm->start();
        auto cameras = cm->cameras();
        if (cameras.empty()) {
            std::cout << "No cameras were identified on the system."
                    << std::endl;
            cm->stop();
            return EXIT_FAILURE;
        }

        string cameraId = cameras[0]->id();
        camera = cm->get(cameraId);
        camera->acquire();
        std::unique_ptr<CameraConfiguration> config = camera->generateConfiguration( { StreamRole::Viewfinder } );
        StreamConfiguration &streamConfig = config->at(0);
        std::cout << "Default viewfinder configuration is: " << streamConfig.toString() << std::endl;
        streamConfig.size.width = 640;
        streamConfig.size.height = 480;
        config->validate();
        std::cout << "Validated viewfinder configuration is: " << streamConfig.toString() << std::endl;
        camera->configure(config.get());
        FrameBufferAllocator *allocator = new FrameBufferAllocator(camera);

        for (StreamConfiguration &cfg : *config) {
            int ret = allocator->allocate(cfg.stream());
            if (ret < 0) {
                std::cerr << "Can't allocate buffers" << std::endl;
                return -ENOMEM;
            }
            size_t allocated = allocator->buffers(cfg.stream()).size();
            std::cout << "Allocated " << allocated << " buffers for stream" << std::endl;
        }

        Stream *stream = streamConfig.stream();
        const std::vector<std::unique_ptr<FrameBuffer>> &buffers = allocator->buffers(stream);
        std::vector<std::unique_ptr<Request>> requests;

        for (unsigned int i = 0; i < buffers.size(); ++i) {
            std::unique_ptr<Request> request = camera->createRequest();
            if (!request)
            {
                std::cerr << "Can't create request" << std::endl;
                return -ENOMEM;
            }
            const std::unique_ptr<FrameBuffer> &buffer = buffers[i];
            int ret = request->addBuffer(stream, buffer.get());
            if (ret < 0)
            {
                std::cerr << "Can't set buffer for request"
                    << std::endl;
                return ret;
            }
            requests.push_back(std::move(request));
        }

        camera->requestCompleted.connect(requestComplete);
        camera->start();
        while(1) {
            for (std::unique_ptr<Request> &request : requests){
                camera->queueRequest(request.get());
                usleep(30*1000);
            }
        }

        return 0; })
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
 * 2. 초기화 이벤트 핸들러로 cameraThread 함수 등록
 * 3. 서버 스레드 시작
 * 4. 무한 루프로 서버 유지
 */
int main(int argc, char *argv[])
{

    RTSPServer::getInstance().setProtocol(Protocol::PROTO_H264);
    RTSPServer::getInstance().onInitEvent = cameraThread;
    RTSPServer::getInstance().startServerThread();

    while(1){
        sleep(1);
    }
}