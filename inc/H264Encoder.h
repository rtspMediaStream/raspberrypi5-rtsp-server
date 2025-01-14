/**
 * @file H264Encoder.h
 * @brief H264 비디오 프레임 파싱 및 인코딩 클래스 헤더
 * @details H264 포맷의 비디오 파일을 NAL(Network Abstraction Layer) Unit으로 
 *          파싱하고 프레임을 추출하는 기능을 제공하는 클래스
 *          - mmap을 사용한 효율적인 파일 접근
 *          - H264 start code 인식 및 처리
 *          - NAL Unit 프레임 추출
 * 
 * @organization rtspMediaStream
 * @repository https://github.com/rtspMediaStream/raspberrypi5-rtsp-server
 * 
 * Copyright (c) 2024 rtspMediaStream
 * This project is licensed under the MIT License - see the LICENSE file for details
 */

#pragma once

#include <utility>

#include <cstddef>
#include <cstdint>

#include <sys/types.h>

//constexpr uint8_t NALU_F_MASK = 0x80;

// NAL Unit Type 관련 상수
constexpr uint8_t NALU_NRI_MASK = 0x60;     ///< NALU 중요도 관련 비트 마스크
constexpr uint8_t NALU_F_NRI_MASK = 0xE0;   ///< NALU 금지 비트와 참조 중요도 마스크
constexpr uint8_t NALU_TYPE_MASK = 0x1F;    ///< NALU 타입 마스크

constexpr uint8_t FU_S_MASK = 0x80;         ///< FU 헤더 시작 비트 마스크
constexpr uint8_t FU_E_MASK = 0x40;         ///< FU 헤더 끝 비트 마스크
constexpr uint8_t SET_FU_A_MASK = 0x1C;     ///< FU 헤더 플래그 비트 마스크

/**
 * @class H264Encoder
 * @brief H264 비디오 프레임을 처리하는 클래스
 * @details H264 포맷의 비디오 파일을 읽고 NAL 단위로 프레임을 추출하는 기능을 제공하는 클래스.
 *          mmap을 사용하여 파일을 메모리에 매핑하고, start code를 찾아 NAL 단위로 프레임을 파싱한다.
 */
class H264Encoder
{
private:
    int fd = -1; /// 파일 디스크립터

    /**
     * @brief 다음 start code의 위치를 찾는 메서드
     * @param _buffer start code를 찾을 버퍼
     * @param buffer_len 버퍼의 길이
     * @return const uint8_t* 다음 start code의 위치를 반환
     */
    static const uint8_t *find_next_start_code(const uint8_t *_buffer, const int64_t buffer_len);

    uint8_t *ptr_mapped_file_cur = nullptr;     ///< 현재 매핑된 파일 위치 포인터
    uint8_t *ptr_mapped_file_start = nullptr;   ///< 매핑된 파일 시작 포인터 
    uint8_t *ptr_mapped_file_end = nullptr;     ///< 매핑된 파일 끝 포인터
    int64_t file_size = 0;                      ///< 파일 크기

public:
    /**
     * @brief 생성자 - 비디오 파일을 열고 메모리에 매핑
     * @param filename 처리할 H264 비디오 파일 경로
     * @throw std::runtime_error 파일 열기 또는 메모리 매핑 실패 시
     */
    explicit H264Encoder(const char *filename);

    /**
     * @brief 소멸자 - 할당된 자원 해제
     */
    ~H264Encoder();

    /**
     * @brief H264 start code 여부를 확인하는 정적 메서드
     * @param _buffer 검사할 버퍼
     * @param buffer_len 버퍼의 길이
     * @param start_code_type start code 타입 (3 또는 4 바이트)
     * @return bool start code 여부
     */
    static bool is_start_code(const uint8_t *_buffer, int64_t buffer_len, uint8_t start_code_type);
    
    /**
     * @brief 다음 H264 프레임을 가져오는 메서드
     * @return std::pair<const uint8_t *, int64_t> 프레임 데이터 포인터와 크기의 쌍
     * @details 
     * - 첫 번째 요소: 프레임 데이터의 시작 포인터 (nullptr if end of file)
     * - 두 번째 요소: 프레임 데이터의 크기 (바이트)
     */
    std::pair<const uint8_t *, int64_t> get_next_frame();
};