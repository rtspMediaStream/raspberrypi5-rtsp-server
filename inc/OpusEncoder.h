
#include <opus/opus.h>
#include <iostream>

const static int OPUS_SAMPLE_RATE = 48000;
const static int OPUS_CHANNELS = 2;
const static int OPUS_FRAME_SIZE = 960;
const static int APPLICATION = OPUS_APPLICATION_AUDIO;
const static int MAX_PACKET_SIZE = 1500;

class OpusEncoder {
private:
    OpusEncoder* encoder;
    unsigned char encoded_buffer[MAX_PACKET_SIZE];
    int error;

public:
    OpusEncoder();

    int encode(const short* pcm_buffer, int frame_size, unsigned char* encoded_data);

    ~OpusEncoder();
};