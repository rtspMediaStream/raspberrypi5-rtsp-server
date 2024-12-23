/**
 * @file DataCapture.cpp
 * @brief DataCapture 클래스의 구현부
 * 
 * Copyright (c) 2024 rtspMediaStream
 * This project is licensed under the MIT License - see the LICENSE file for details
 */

#include "DataCapture.h"
#include <cstring>
#include <iostream>

void DataCapture::pushFrame(const DataCaptureFrame& frame)
{
    if(!isFullBuffer()){
        bufferMutex.lock();
        if(frameBuffer[tail].dataPtr == nullptr){
            frameBuffer[tail].dataPtr = new unsigned char[frame.size];
        }else if(frameBuffer[tail].size < frame.size){
            delete[] frameBuffer[tail].dataPtr;
            frameBuffer[tail].dataPtr = new unsigned char[frame.size];
        }

        memcpy(frameBuffer[tail].dataPtr, frame.dataPtr, frame.size);
        frameBuffer[tail].size = frame.size;
        frameBuffer[tail].timestamp = frame.timestamp;


        tail = (tail + 1) % buffer_max_size;
        buffer_size++;
        // std::cout << "push img \n"<<buffer_size;
        bufferMutex.unlock();
    }
}

DataCaptureFrame DataCapture::popFrame() 
{
    bufferMutex.lock();
    if(isEmptyBuffer()) {
        bufferMutex.unlock();
        return {nullptr, 0, 0};
    }

    auto ret = frameBuffer[head];
    head = (head + 1) % buffer_max_size; // head 위치 갱신

    // std::cout << "pop  img \n"<<buffer_size;
    buffer_size--;
    bufferMutex.unlock();

    return ret;
}