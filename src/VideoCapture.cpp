#include "VideoCapture.h"
#include <cstring>
#include <iostream>

void VideoCapture::pushImg(const VCImage& img) {
    if(!isFullBuffer()){
        imgBufferMutex.lock();
        if(imgBuffer[tail].img == nullptr){
            imgBuffer[tail].img = new unsigned char[img.size];
        }else if(imgBuffer[tail].size < img.size){
            delete[] imgBuffer[tail].img;
            imgBuffer[tail].img = new unsigned char[img.size];
        }

        memcpy(imgBuffer[tail].img, img.img, img.size);
        imgBuffer[tail].size = img.size;
        imgBuffer[tail].timestamp = img.timestamp;


        tail = (tail + 1) % buffer_max_size; // tail 위치 갱신
        buffer_size++;
        // std::cout << "push img \n"<<buffer_size;
        imgBufferMutex.unlock();
    }
}

VCImage VideoCapture::popImg() {
    imgBufferMutex.lock();
    if(isEmptyBuffer()) {
        imgBufferMutex.unlock();
        return {nullptr, 0, 0};
    }

    auto ret = imgBuffer[head];
    head = (head + 1) % buffer_max_size; // head 위치 갱신

    // std::cout << "pop  img \n"<<buffer_size;
    buffer_size--;
    imgBufferMutex.unlock();

    return ret;
}