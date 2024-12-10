#include "VideoCapture.h"
#include <cstring>

void VideoCapture::pushImg(unsigned char* imgPtr, int size) {

    if(!isFullBuffer()){
        std::lock_guard<std::mutex> lock(imgBufferMutex);

        if(imgBuffer[tail].first == nullptr){
            imgBuffer[tail].first = new unsigned char[size];
        }else if(imgBuffer[tail].second < size){
            delete[] imgBuffer[tail].first;
            imgBuffer[tail].first = new unsigned char[size];
        }

        memcpy(imgBuffer[tail].first, imgPtr, size);
        imgBuffer[tail].second = size;


        tail = (tail + 1) % buffer_max_size; // tail 위치 갱신
        buffer_size++;
    }
}

std::pair<unsigned char*, int> VideoCapture::popImg() {
    if (isEmptyBuffer()) {
        return std::make_pair(nullptr, 0);
    }

    std::lock_guard<std::mutex> lock(imgBufferMutex);

    auto ret = imgBuffer[head];
    head = (head + 1) % buffer_max_size; // head 위치 갱신
    buffer_size--;

    return ret;
}