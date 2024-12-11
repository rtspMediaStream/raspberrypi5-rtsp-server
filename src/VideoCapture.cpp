#include "VideoCapture.h"
#include <cstring>
#include <iostream>

void VideoCapture::pushImg(unsigned char* imgPtr, int size) {

    if(!isFullBuffer()){
        imgBufferMutex.lock();
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
        std::cout << "push img \n"<<buffer_size;
        imgBufferMutex.unlock();
    }
}

std::pair<unsigned char*, int> VideoCapture::popImg() {
    if (isEmptyBuffer()) {
        return std::make_pair(nullptr, 0);
    }

    imgBufferMutex.lock();
    auto ret = imgBuffer[head];
    head = (head + 1) % buffer_max_size; // head 위치 갱신

    std::cout << "pop  img \n"<<buffer_size;
    buffer_size--;
    imgBufferMutex.unlock();

    return ret;
}