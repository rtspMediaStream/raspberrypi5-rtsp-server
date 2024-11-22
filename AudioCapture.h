#include <alsa/asoundlib.h>

class AudioCapture {
private:
    snd_pcm_t* pcm_handle;
    snd_pcm_hw_params_t* params;
    unsigned int sample_rate;
    int dir;

public:
    AudioCapture(unsigned int rate = 48000);

    int read(short *buffer, int frames);

    ~AudioCapture();
};