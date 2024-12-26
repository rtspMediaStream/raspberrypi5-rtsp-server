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

/**
 * @struct DataCaptureFrame
 * @brief 캡처된 프레임 데이터를 저장하는 구조체
 */
struct DataCaptureFrame {
    unsigned char *dataPtr; /// 프레임 데이터 포인터
    unsigned int size; /// 프레임 데이터 크기
    unsigned int timestamp; /// 프레임 타임스탬프
};

/**
 * @class DataCapture
 * @brief 미디어 프레임 캡처 및 버퍼 관리를 위한 싱글톤 클래스
 * @details 순환 큐 기반의 프레임 버퍼 관리 시스템을 구현한 클래스로,
 *          프레임 데이터를 캡처하고 버퍼에 저장하며, 필요 시 프레임 데이터를 반환하는 기능을 제공한다.
 */
class DataCapture {
public:
    static const int buffer_max_size = 10; /// 프레임 버퍼 최대 크기

    /**
     * @brief 싱글톤 인스턴스를 반환하는 정적 메서드
     * @return DataCapture& 싱글톤 인스턴스에 대한 참조
     */
    static DataCapture &getInstance()
    {
        static DataCapture instance;
        return instance;
    }

    /**
     * @brief 버퍼가 비어있는지 확인
     * @return true 버퍼가 비어있는 경우
     * @return false 버퍼에 데이터가 있는 경우
     */
    inline bool isEmptyBuffer() { return (buffer_size == 0); };
    /**
     * @brief 버퍼가 가득 찼는지 확인
     * @return true 버퍼가 가득 찬 경우
     * @return false 버퍼에 여유 공간이 있는 경우
     */
    inline bool isFullBuffer() { return (buffer_size == buffer_max_size); };
    
    /**
     * @brief 프레임 데이터를 버퍼에 저장하는 메서드
     * @param frame 저장할 프레임 데이터
     */
    virtual void pushFrame(const DataCaptureFrame& frame);
    
    /**
     * @brief 버퍼에서 프레임 데이터를 반환하는 메서드
     * @return DataCaptureFrame 버퍼에서 반환된 프레임 데이터
     */
    virtual DataCaptureFrame popFrame();

protected:
    std::vector <DataCaptureFrame> frameBuffer; /// 프레임 데이터를 저장하는 버퍼
    int head = 0; /// 버퍼의 시작 위치
    int tail = 0; /// 버퍼의 끝 위치
    int buffer_size = 0; /// 버퍼에 저장된 프레임 수

    std::mutex bufferMutex; /// 버퍼에 대한 스레드 안전성을 보장하는 뮤텍스

    /**
     * @brief 생성자 - 버퍼 초기화
     */
    DataCapture()
    {
        frameBuffer.resize(buffer_max_size, {nullptr, 0 , 0});
    }

    /**
     * @brief 소멸자 - 할당된 메모리 해제
     */
    ~DataCapture()
    {
        for(auto& pair : frameBuffer) {
            delete[] pair.dataPtr;
        }
    }
};

#endif //__DATACAPTURE_H__