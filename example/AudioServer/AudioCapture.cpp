/**
* @file AudioCapture.cpp
* @brief AudioCapture 클래스의 구현부
* @details ALSA를 사용하여 오디오 입력을 캡처하고 Opus 인코딩을 위한 설정을 제공하는 클래스
* 
* @organization rtspMediaStream
* @repository https://github.com/rtspMediaStream/raspberrypi5-rtsp-server
* 
* Copyright (c) 2024 rtspMediaStream
* This project is licensed under the MIT License - see the LICENSE file for details
*/

#include "AudioCapture.h"
#include "OpusEncoder.h"
#include <opus/opus.h>
#include <iostream>

/**
* @brief AudioCapture 생성자
* @details 
*   - ALSA PCM 디바이스를 캡처 모드로 열기
*   - Opus 인코딩에 적합한 오디오 파라미터 설정:
*     - 48kHz 샘플링 레이트
*     - 16비트 리틀 엔디안 포맷
*     - 스테레오 채널
*     - 인터리브드 접근 방식
* @throw std::runtime_error PCM 디바이스 열기 또는 하드웨어 파라미터 설정 실패 시
*/
AudioCapture::AudioCapture()
{
    sample_rate = OPUS_SAMPLE_RATE;
    int rc = snd_pcm_open(&pcm_handle, "default", SND_PCM_STREAM_CAPTURE, 0);
    if (rc < 0)
    {
        throw std::runtime_error("PCM 디바이스를 열 수 없습니다: " + std::string(snd_strerror(rc)));
    }
    
    snd_pcm_hw_params_alloca(&params);
    snd_pcm_hw_params_any(pcm_handle, params);
    snd_pcm_hw_params_set_access(pcm_handle, params, SND_PCM_ACCESS_RW_INTERLEAVED);
    snd_pcm_hw_params_set_format(pcm_handle, params, SND_PCM_FORMAT_S16_LE);
    snd_pcm_hw_params_set_rate(pcm_handle, params, OPUS_SAMPLE_RATE, 0);
    snd_pcm_hw_params_set_channels(pcm_handle, params, OPUS_CHANNELS);
    snd_pcm_hw_params_set_period_size(pcm_handle, params, OPUS_FRAME_SIZE, 0);  
    rc = snd_pcm_hw_params(pcm_handle, params);
    if (rc < 0)
    {
        throw std::runtime_error("하드웨어 파라미터를 설정할 수 없습니다: " + std::string(snd_strerror(rc)));
    }
}

/**
* @brief PCM 디바이스에서 오디오 데이터를 읽는 메서드
* @param buffer 오디오 데이터를 저장할 버퍼
* @param frames 읽을 프레임 수
* @return int 성공적으로 읽은 프레임 수 또는 오류 코드
* @details
*   - PCM 디바이스로부터 지정된 프레임 수만큼 오디오 데이터를 읽음
*   - 오버런 발생 시 자동으로 복구 시도
*   - 오류 발생 시 오류 메시지 출력
*/
int AudioCapture::read(short *buffer, int frames)
{
    int rc = snd_pcm_readi(pcm_handle, buffer, frames);
    if (rc == -EPIPE)
    {
        std::cerr << "오버런 발생" << std::endl;
        snd_pcm_prepare(pcm_handle);
    }
    else if (rc < 0)
    {
        std::cerr << "PCM 디바이스에서 읽기 오류: " << snd_strerror(rc) << std::endl;
    }
    return rc;
}

/**
* @brief AudioCapture 소멸자
* @details PCM 디바이스를 안전하게 닫고 리소스를 해제
*/
AudioCapture::~AudioCapture()
{
    snd_pcm_close(pcm_handle);
}