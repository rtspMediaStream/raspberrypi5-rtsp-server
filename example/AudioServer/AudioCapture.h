#include "DataCapture.h"
#include <alsa/asoundlib.h>
#include <queue>
#include <mutex>

class AudioCapture : public DataCapture{
public:
    static AudioCapture& getInstance() {
        static AudioCapture instance;
        return instance;
    }

    AudioCapture();
    int read(short *buffer, int frames);
    ~AudioCapture();
private:
    std::queue <std::pair<unsigned char*, int>> buffer;
    std::mutex bufferMutex;
    snd_pcm_t* pcm_handle;
    snd_pcm_hw_params_t* params;
    unsigned int sample_rate;
    int dir;
};