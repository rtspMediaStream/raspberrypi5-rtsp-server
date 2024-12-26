/**
 * @file DataCapture.cpp
 * @brief DataCapture 클래스의 구현부
 * @details DataCapture 클래스의 멤버 함수를 구현한 소스 파일
 * 
 * Copyright (c) 2024 rtspMediaStream
 * This project is licensed under the MIT License - see the LICENSE file for details
 */

#include "DataCapture.h"
#include <cstring>
#include <iostream>

/**
 * @brief 프레임을 버퍼에 추가하는 메서드
 * @param frame 추가할 프레임 데이터
 * @details
 *   - 버퍼가 가득 차지 않은 경우만 프레임 추가
 *   - 필요시 새로운 메모리 할당 또는 기존 메모리 재사용
 *   - 스레드 안전성을 위한 뮤텍스 사용
 */
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

/**
 * @brief 버퍼에서 프레임을 추출하는 메서드
 * @return DataCaptureFrame 추출된 프레임 데이터
 * @details
 *   - 버퍼가 비어있는 경우 NULL 반환
 *   - FIFO 방식으로 가장 오래된 프레임을 반환
 *   - 스레드 안전성을 위한 뮤텍스 사용
 */
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