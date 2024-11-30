#include "OpusEncoder.h"

OpusEncoder::OpusEncoder()
{
    encoder = opus_encoder_create(OPUS_SAMPLE_RATE, OPUS_CHANNELS, APPLICATION, &error);
    if (error != OPUS_OK)
    {
        throw std::runtime_error("Opus 인코더 생성 실패: " + std::string(opus_strerror(error)));
    }

    // 비트레이트 설정 (128kbps)
    opus_encoder_ctl(encoder, OPUS_SET_BITRATE(128000));
    opus_encoder_ctl(encoder, OPUS_SET_COMPLEXITY(10));  // 최대 복잡도로 설정
}

int OpusEncoder::encode(const short *pcm_buffer, int frame_size, unsigned char *encoded_data)
{
    int encoded_bytes = opus_encode(encoder, pcm_buffer, frame_size,
                                    encoded_data, MAX_PACKET_SIZE);
    if (encoded_bytes < 0)
    {
        throw std::runtime_error("Opus 인코딩 실패: " + std::string(opus_strerror(encoded_bytes)));
    }
    return encoded_bytes;
}

OpusEncoder::~OpusEncoder()
{
    if (encoder)
    {
        opus_encoder_destroy(encoder);
    }
}