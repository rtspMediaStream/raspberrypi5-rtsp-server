#include "utils.h"
#include "ClientSession.h"
#include "TCPHandler.h"
#include "UDPHandler.h"
#include "RequestHandler.h"
#include "MediaStreamHandler.h"
#include "global.h"
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

void showHelp(const char *thisName);
bool isPrivilegedPort(int port);
bool isRunningAsRoot();
void signalHandler(int signal);

void encode_video() {
    // FFmpeg 5.x는 자동 초기화를 제공 (av_register_all() 필요 없음)
    AVFormatContext *formatContext = nullptr;
    AVCodecContext *codecContext = nullptr;
    const AVCodec *codec = nullptr;
    AVStream *stream = nullptr;
    AVFrame *frame = nullptr;
    int ret;

    // H.264 코덱 찾기
    codec = avcodec_find_encoder(AV_CODEC_ID_H264);
    if (!codec) {
        std::cerr << "H.264 encoder not found!" << std::endl;
        return;
    }

    // 코덱 컨텍스트 초기화
    codecContext = avcodec_alloc_context3(codec);
    if (!codecContext) {
        std::cerr << "Failed to allocate codec context!" << std::endl;
        return;
    }

    // 코덱 설정
    codecContext->codec_id = AV_CODEC_ID_H264;
    codecContext->codec_type = AVMEDIA_TYPE_VIDEO;
    codecContext->width = 640;
    codecContext->height = 480;
    codecContext->pix_fmt = AV_PIX_FMT_YUV420P; // H.264는 YUV420P를 기본적으로 사용
    codecContext->time_base = {1, 30};          // 30 FPS
    codecContext->bit_rate = 2000000;           // 2 Mbps 비트레이트

    // AVFormatContext 생성
    ret = avformat_alloc_output_context2(&formatContext, nullptr, "mp4", "output.mp4");
    if (!formatContext || ret < 0) {
        std::cerr << "Failed to allocate format context!" << std::endl;
        avcodec_free_context(&codecContext);
        return;
    }

    // 스트림 추가
    stream = avformat_new_stream(formatContext, nullptr);
    if (!stream) {
        std::cerr << "Failed to create video stream!" << std::endl;
        avcodec_free_context(&codecContext);
        avformat_free_context(formatContext);
        return;
    }
    stream->time_base = codecContext->time_base;

    // 스트림 코덱 파라미터 설정
    ret = avcodec_parameters_from_context(stream->codecpar, codecContext);
    if (ret < 0) {
        std::cerr << "Failed to copy codec parameters to stream!" << std::endl;
        avcodec_free_context(&codecContext);
        avformat_free_context(formatContext);
        return;
    }

    // 코덱 열기
    ret = avcodec_open2(codecContext, codec, nullptr);
    if (ret < 0) {
        std::cerr << "Failed to open codec!" << std::endl;
        avcodec_free_context(&codecContext);
        avformat_free_context(formatContext);
        return;
    }

    // 출력 파일 열기
    if (!(formatContext->flags & AVFMT_NOFILE)) {
        ret = avio_open(&formatContext->pb, "output.mp4", AVIO_FLAG_WRITE);
        if (ret < 0) {
            std::cerr << "Failed to open output file!" << std::endl;
            avcodec_free_context(&codecContext);
            avformat_free_context(formatContext);
            return;
        }
    }

    // 포맷 헤더 쓰기
    ret = avformat_write_header(formatContext, nullptr);
    if (ret < 0) {
        std::cerr << "Error occurred when writing header!" << std::endl;
        avcodec_free_context(&codecContext);
        avformat_free_context(formatContext);
        avio_close(formatContext->pb);
        return;
    }

    // 프레임 생성 및 초기화
    frame = av_frame_alloc();
    if (!frame) {
        std::cerr << "Failed to allocate frame!" << std::endl;
        avcodec_free_context(&codecContext);
        avformat_free_context(formatContext);
        avio_close(formatContext->pb);
        return;
    }

    frame->format = codecContext->pix_fmt;
    frame->width = codecContext->width;
    frame->height = codecContext->height;

    ret = av_frame_get_buffer(frame, 32); // 32-byte alignment
    if (ret < 0) {
        std::cerr << "Failed to allocate frame buffer!" << std::endl;
        av_frame_free(&frame);
        avcodec_free_context(&codecContext);
        avformat_free_context(formatContext);
        avio_close(formatContext->pb);
        return;
    }

    // --- 인코딩 루프 작성 가능 ---
    // 여기에 실제 데이터를 frame->data에 채워 넣고,
    // avcodec_send_frame 및 avcodec_receive_packet을 호출하여 패킷을 생성 및 출력.
    // ---------------------------

    // 리소스 정리
    av_frame_free(&frame);
    avcodec_free_context(&codecContext);
    avformat_free_context(formatContext);
    if (!(formatContext->flags & AVFMT_NOFILE)) {
        avio_close(formatContext->pb);
    }

    std::cout << "Video encoding completed successfully!" << std::endl;
}

FFmpegEncoder ffmpegEncoder("output.h264",640, 480, 30.0);

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
                ServerStream::getInstance().type = ServerStreamType::Audio;
            }else if(mode == "Video") {
                ServerStream::getInstance().type = ServerStreamType::Video;
            }else if(mode == "Camera") {
                ServerStream::getInstance().type = ServerStreamType::Camera;
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
     std::thread([]()->int {
         //camera open
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

         return 0;
     }).detach();
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