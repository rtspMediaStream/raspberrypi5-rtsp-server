#include "RTSPServer.h"
#include <unistd.h>
#include <iostream>

#include <thread>
#include "H264Encoder.h"
#include "DataCapture.h"

// 파일에서 VideoCapture Queue로 던지기
void LoadH264File()
{
    std::thread([]() -> void
                {
                std::cout << "thread start"<<std::endl;
            constexpr int64_t target_frame_duration_us = 1000000 / 30; // 30 fps -> 33,333 microseconds per frame
            H264Encoder* h264_file = new H264Encoder("../dragon.h264");

            DataCaptureFrame image;
            while (true) {
                auto frame_start_time = std::chrono::high_resolution_clock::now();

                // Get the next frame
                std::pair<const uint8_t *, int64_t> cur_frame = h264_file->get_next_frame();

                if (cur_frame.first == nullptr) {
                    return;
                }
                image.dataPtr = (unsigned char*)cur_frame.first;
                image.size = cur_frame.second;
                image.timestamp += 3000;

                // Process the frame
                DataCapture::getInstance().pushFrame(image);

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

int main(int argc, char *argv[])
{
    
    RTSPServer::getInstance().setProtocol(Protocol::PROTO_H264);
    RTSPServer::getInstance().startServerThread();

    RTSPServer::getInstance().onInitEvent = LoadH264File;

    while (1)
    {
        sleep(1);
    }
    
        
    return 0;
}