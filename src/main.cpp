#include "utils.h"
#include "ClientSession.h"
#include "TCPHandler.h"
#include "UDPHandler.h"
#include "RequestHandler.h"
#include "MediaStreamHandler.h"
#include "global.h"
#include "event_loop.h"
#include "VideoCapture.h"

#include <string>
#include <thread>
#include <iostream>
#include <csignal>
#include <vector>
#include <fstream>
#include <iomanip>
#include <memory>
#include <sys/mman.h> // mmap, munmap

/// camera check in raspberry pi
#include <opencv2/opencv.hpp>
#include <libcamera/libcamera.h>
using namespace libcamera;
using namespace std;
#define TIMEOUT_SEC 30
static std::shared_ptr<Camera> camera;
static EventLoop loop;

void showHelp(const char *thisName);
bool isPrivilegedPort(int port);
bool isRunningAsRoot();
void signalHandler(int signal);

static void processRequest(Request *request)
{
    if (request->status() == Request::RequestCancelled) {
        std::cerr << "Request was cancelled!" << std::endl;
        return;
    }

    // Request에서 스트림과 버퍼를 가져옴
    VCImage newframe;
    for (const auto &bufferPair : request->buffers()) {
        const FrameBuffer *buffer = bufferPair.second;

        // 버퍼의 첫 번째 Plane 가져오기
        const FrameBuffer::Plane &plane = buffer->planes()[0];

        // fd를 사용하여 메모리 매핑
        void *mappedMemory = mmap(nullptr, plane.length, PROT_READ, MAP_SHARED, plane.fd.get(), 0);
        if (mappedMemory == MAP_FAILED) {
            std::cerr << "Failed to mmap buffer!" << std::endl;
            continue;
        }

        // 이미지 데이터를 unsigned char*로 처리
        unsigned char *imgData = static_cast<unsigned char *>(mappedMemory);
        int imgSize = plane.length;

        // 이미지 데이터를 pushImg 함수로 전달
        newframe.img = imgData;
        newframe.size = imgSize;
        newframe.timestamp += 3000;
        VideoCapture::getInstance().pushImg(newframe);
        std::cout << "Image data processed, size: " << imgSize << " bytes" << std::endl;

        // 메모리 매핑 해제
        munmap(mappedMemory, plane.length);
    }
}

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

        // 포맷에 따라 OpenCV 처리
        int width = 640;  // 기본값: 640x480
        int height = 480;

        // XRGB8888 -> RGB888 변환
        uint8_t *src = static_cast<uint8_t *>(mappedMemory);
        cv::Mat rgbImage(height, width, CV_8UC3); // RGB888 형식의 OpenCV Mat

        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                uint32_t pixel = reinterpret_cast<uint32_t *>(src)[y * width + x]; // XRGB8888 한 픽셀 읽기
                uint8_t r = (pixel >> 16) & 0xFF; // R 추출
                uint8_t g = (pixel >> 8) & 0xFF;  // G 추출
                uint8_t b = pixel & 0xFF;         // B 추출

                // RGB888 포맷으로 저장
                rgbImage.at<cv::Vec3b>(y, x) = cv::Vec3b(b, g, r); // OpenCV는 BGR 순서
            }
        }
        
        // 이미지 파일로 저장
        std::ostringstream fileName;
        fileName << "frame_" << std::setw(6) << std::setfill('0') << metadata.sequence << ".png";
        if (cv::imwrite(fileName.str(), rgbImage)) {
            std::cout << "Saved image: " << fileName.str() << std::endl;
        } else {
            std::cerr << "Failed to save image: " << fileName.str() << std::endl;
        }

        // 매핑 해제
        munmap(mappedMemory, length);
    }

    request->reuse(Request::ReuseBuffers);
}


int main(int argc, char* argv[]) {
    int option;
    extern char *optarg;

    if(argc < 2){
        showHelp(argv[0]);
        return 1;
    }
    while((option = getopt(argc, argv, "hm:")) != -1)
    {
        switch(option)
        {
        case 'h':
            showHelp(argv[0]);
            return 1;
        case 'm':
            std::string mode(optarg);
            if(mode == "Audio") {
                ServerStream::getInstance().type = Audio;
            }else if(mode == "Video") {
                ServerStream::getInstance().type = Video;
            }
            break;
        }
    }

    if(isPrivilegedPort(g_serverRtpPort) && !isRunningAsRoot()) {
        std::cerr << "Error: Program must be run as root to bind to privileged ports.\n";
        return 1;
    };

    std::cout << "Start RTSP server (" << ServerStream::getInstance().type << ")" << std::endl;
    signal(SIGINT, signalHandler);

    ///camera test
    // std::thread([]()->int {
    //     //camera open
    //     static std::shared_ptr<Camera> camera;
    //     std::unique_ptr<CameraManager> cm = std::make_unique<CameraManager>();
    //     cm->start();
    //     auto cameras = cm->cameras();
    //     if (cameras.empty()) {
    //         std::cout << "No cameras were identified on the system."
    //                 << std::endl;
    //         cm->stop();
    //         return EXIT_FAILURE;
    //     }

    //     string cameraId = cameras[0]->id();
    //     camera = cm->get(cameraId);
    //     camera->acquire();
    //     std::unique_ptr<CameraConfiguration> config = camera->generateConfiguration( { StreamRole::Viewfinder } );
    //     StreamConfiguration &streamConfig = config->at(0);
    //     std::cout << "Default viewfinder configuration is: " << streamConfig.toString() << std::endl;
    //     streamConfig.size.width = 640;
    //     streamConfig.size.height = 480;
    //     config->validate();
    //     std::cout << "Validated viewfinder configuration is: " << streamConfig.toString() << std::endl;
    //     camera->configure(config.get());
    //     FrameBufferAllocator *allocator = new FrameBufferAllocator(camera);

    //     for (StreamConfiguration &cfg : *config) {
    //         int ret = allocator->allocate(cfg.stream());
    //         if (ret < 0) {
    //             std::cerr << "Can't allocate buffers" << std::endl;
    //             return -ENOMEM;
    //         }

    //         size_t allocated = allocator->buffers(cfg.stream()).size();
    //         std::cout << "Allocated " << allocated << " buffers for stream" << std::endl;
    //     }

    //     Stream *stream = streamConfig.stream();
    //     const std::vector<std::unique_ptr<FrameBuffer>> &buffers = allocator->buffers(stream);
    //     std::vector<std::unique_ptr<Request>> requests;

    //     for (unsigned int i = 0; i < buffers.size(); ++i) {
    //         std::unique_ptr<Request> request = camera->createRequest();
    //         if (!request)
    //         {
    //             std::cerr << "Can't create request" << std::endl;
    //             return -ENOMEM;
    //         }

    //         const std::unique_ptr<FrameBuffer> &buffer = buffers[i];
    //         int ret = request->addBuffer(stream, buffer.get());
    //         if (ret < 0)
    //         {
    //             std::cerr << "Can't set buffer for request"
    //                 << std::endl;
    //             return ret;
    //         }

    //         requests.push_back(std::move(request));
    //     }

    //     camera->requestCompleted.connect(requestComplete);

    //     camera->start();
    //     while(1) {
    //         for (std::unique_ptr<Request> &request : requests){
    //             camera->queueRequest(request.get());
    //             usleep(30*1000 * 1000000);
    //         }
    //     }

    //     return 0;
    // }).detach();
    //camera end

    while (true) {
        std::string newIp;
        int newClient = TCPHandler::GetInstance().AcceptClientConnection(newIp);
        if(newClient == -1){
            break;
        }else {
            std::cout << "Client Ip:" << newIp << " connected." << std::endl;
        }
        ClientSession* clientSession = new ClientSession(newClient, newIp);

        RequestHandler requestHandler(clientSession);
        requestHandler.StartThread();
    }
    
    std::cout << "Stop RTSP server" << std::endl;
    return 0;
}

void showHelp(const char *thisName) {
    printf("usage : %s [OPTION]\n"
           " -h            help\n"
           " -m [media type] Audio or Video\n",
           thisName);
}

bool isPrivilegedPort(int port) {
    return port <= 1024;
}
bool isRunningAsRoot() {
    return getuid() == 0;
}

void signalHandler(int signal) {
    std::cout << "Interrupt signal (" << signal << ") received. Shutting down..." << std::endl;
    TCPHandler::GetInstance().CloseClientConnection();
}