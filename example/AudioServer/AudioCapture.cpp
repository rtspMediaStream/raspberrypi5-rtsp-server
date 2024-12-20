#include "AudioCapture.h"
#include "OpusEncoder.h"
#include <opus/opus.h>
#include <iostream>

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
    //snd_pcm_hw_params_set_rate_near(pcm_handle, params, &sample_rate, &dir);
    rc = snd_pcm_hw_params(pcm_handle, params);
    if (rc < 0)
    {
        throw std::runtime_error("하드웨어 파라미터를 설정할 수 없습니다: " + std::string(snd_strerror(rc)));
    }
}

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

AudioCapture::~AudioCapture()
{
    snd_pcm_close(pcm_handle);
}