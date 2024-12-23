/**
 * @file DataCapture.h
 * @brief 미디어 프레임 캡처 및 버퍼 관리 클래스 헤더
 * @details 미디어 스트리밍을 위한 프레임 캡처 및 버퍼 관리 시스템을 제공하는 클래스
 *          - 순환 큐를 사용한 프레임 데이터 관리
 *          - 스레드를 통한 안전한 프레임 데이터 접근
 *          - FIFO 방식의 프레임 처리
 * 
 * @organization rtspMediaStream
 * @repository https://github.com/rtspMediaStream/raspberrypi5-rtsp-server
 * 
 * Copyright (c) 2024 rtspMediaStream
 * This project is licensed under the MIT License - see the LICENSE file for details
 */

#ifndef __DATACAPTURE_H__
#define __DATACAPTURE_H__
#include <vector>
#include <queue>
#include <mutex>

struct DataCaptureFrame {
    unsigned char *dataPtr;
    unsigned int size;
    unsigned int timestamp;
};

class DataCapture {
public:
    static const int buffer_max_size = 10;
    static DataCapture &getInstance()
    {
        static DataCapture instance;
        return instance;
    }
    inline bool isEmptyBuffer() { return (buffer_size == 0); };
    inline bool isFullBuffer() { return (buffer_size == buffer_max_size); };
    virtual void pushFrame(const DataCaptureFrame& frame);
    virtual DataCaptureFrame popFrame();
protected:
    std::vector <DataCaptureFrame> frameBuffer;
    int head = 0;
    int tail = 0;
    int buffer_size = 0;

    std::mutex bufferMutex;
    DataCapture()
    {
        frameBuffer.resize(buffer_max_size, {nullptr, 0 , 0});
    }

    ~DataCapture()
    {
        for(auto& pair : frameBuffer) {
            delete[] pair.dataPtr;
        }
    }
};

#endif //__DATACAPTURE_H__