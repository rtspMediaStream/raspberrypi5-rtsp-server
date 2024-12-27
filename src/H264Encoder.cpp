/**
 * @file H264Encoder.cpp
 * @brief H264Encoder 클래스의 구현부
 * @details H264Encoder 클래스의 멤버 함수를 구현한 소스 파일
 * 
 * Copyright (c) 2024 rtspMediaStream
 * This project is licensed under the MIT License - see the LICENSE file for details
 */

#include "H264Encoder.h"

#include <cassert>
#include <cerrno>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <iostream>

#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>


/**
 * @brief H264Encoder 클래스 생성자
 * @param filename 처리할 H264 비디오 파일 경로
 * @details 
 *   - 파일을 읽기 전용으로 열기
 *   - 파일 크기 확인
 *   - mmap을 사용하여 파일을 메모리에 매핑
 *   - 초기 포인터 설정
 * @throw std::runtime_error 파일 열기 또는 메모리 매핑 실패 시
 */
H264Encoder::H264Encoder(const char *filename)
{
    this->fd = open(filename, O_RDONLY);
    assert(this->fd >= 0);

    struct stat file_stat;
    fstat(this->fd, &file_stat);

    this->file_size = file_stat.st_size;
    this->ptr_mapped_file_start = this->ptr_mapped_file_cur = reinterpret_cast<uint8_t *>(mmap(nullptr, file_size, PROT_READ, MAP_PRIVATE, this->fd, 0));
    this->ptr_mapped_file_end = this->ptr_mapped_file_start + this->file_size;
    assert(this->ptr_mapped_file_cur != MAP_FAILED);
}

/**
 * @brief H264Encoder 클래스 소멸자
 * @details 
 *   - 매핑된 메모리 해제
 *   - 파일 디스크립터 닫기
 */
H264Encoder::~H264Encoder()
{
    assert(close(this->fd) == 0);
    assert(munmap(reinterpret_cast<void *>(this->ptr_mapped_file_start), this->file_size) == 0);
    this->ptr_mapped_file_cur = this->ptr_mapped_file_start = this->ptr_mapped_file_end = nullptr;
}

/**
 * @brief H264 start code 여부를 확인하는 정적 메서드
 * @param _buffer 검사할 버퍼
 * @param buffer_len 버퍼의 길이
 * @param start_code_type start code 타입 (3 또는 4 바이트)
 * @return bool start code 발견 여부
 * @details H264 start code는 다음 두 가지 형태 중 하나:
 *   - 3바이트: 0x000001
 *   - 4바이트: 0x00000001
 */
bool H264Encoder::is_start_code(const uint8_t *_buffer, const int64_t buffer_len, const uint8_t start_code_type)
{
    switch (start_code_type){
    case 3:
        if (buffer_len < 3)
            break;
        return ((_buffer[0] == 0x00) && (_buffer[1] == 0x00) && (_buffer[2] == 0x01));
    case 4:
        if (buffer_len < 4)
            break;
        return ((_buffer[0] == 0x00) && (_buffer[1] == 0x00) && (_buffer[2] == 0x00) && (_buffer[3] == 0x01));
    default:
        fprintf(stderr, "static H264Encoder::is_start_code() failed: start_code_type error\n");
        break;
    }
    return false;
}

/**
 * @brief 다음 start code의 위치를 찾는 정적 메서드
 * @param _buffer 검색할 버퍼의 시작 포인터
 * @param buffer_len 버퍼의 길이
 * @return const uint8_t* 다음 start code의 위치를 가리키는 포인터 (없으면 nullptr)
 * @details 버퍼 내에서 3바이트 또는 4바이트 start code를 순차적으로 검색
 */
const uint8_t *H264Encoder::find_next_start_code(const uint8_t *_buffer, const int64_t buffer_len)
{
    for (int64_t i = 0; i < buffer_len - 3; i++) {
        if (H264Encoder::is_start_code(_buffer, buffer_len - i, 3) || H264Encoder::is_start_code(_buffer, buffer_len - i, 4))
            return _buffer;
        ++_buffer;
    }
    // return nullptr represents reaching the end of this video
    return H264Encoder::is_start_code(_buffer, 3, 3) ? _buffer : nullptr;
}

/**
 * @brief 다음 H264 프레임을 가져오는 메서드
 * @return std::pair<const uint8_t *, int64_t> 프레임 데이터 포인터와 크기의 쌍
 * @details 파일에서 다음 NAL 유닛(Network Abstraction Layer)을 찾아 반환
 *   - 정상적인 경우: {프레임 포인터, 프레임 크기} 반환
 *   - 파일 끝: {nullptr, 0} 반환
 *   - 잘못된 start code: {nullptr, -1} 반환
 */
std::pair<const uint8_t *, int64_t> H264Encoder::get_next_frame()
{
    // assert(this->fd > 0);
    auto remain_bytes = this->ptr_mapped_file_end - this->ptr_mapped_file_cur;
    if (remain_bytes == 0)
        return {nullptr, 0};

    if (!H264Encoder::is_start_code(this->ptr_mapped_file_cur, remain_bytes, 4) && !H264Encoder::is_start_code(this->ptr_mapped_file_cur, remain_bytes, 3)) {
        fprintf(stderr, "H264Encoder::get_one_frame() failed: H264 file not start with startcode\n");
        return {nullptr, -1};
    }

    const uint8_t *ptr_next_start_code = H264Encoder::find_next_start_code(this->ptr_mapped_file_cur + 3, remain_bytes - 3);
    // Reach the end of this H264 video
    if (!ptr_next_start_code)
        return {nullptr, 0};
    const int64_t frame_size = ptr_next_start_code - this->ptr_mapped_file_cur;
    auto ptr_ret = this->ptr_mapped_file_cur;
    this->ptr_mapped_file_cur += frame_size;
    return {ptr_ret, frame_size};
}