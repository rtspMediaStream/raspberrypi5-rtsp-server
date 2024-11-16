#ifndef RTSP_MEDIASTREAMHANDLER_H
#define RTSP_MEDIASTREAMHANDLER_H

#include <atomic>
#include <alsa/asoundlib.h>
#include <condition_variable>

class MediaStreamHandler {
public:
    MediaStreamHandler();

    void HandleMediaStream();

    unsigned char LinearToUlaw(int sample);
    void InitAlsa(snd_pcm_t*& pcmHandle, snd_pcm_hw_params_t*& params, int& rc, unsigned int& sampleRate, int& dir);
    int CaptureAudio(snd_pcm_t*& pcmHandle, short*& buffer, int& frames, int& rc, unsigned char*& payload);

    void SetCmd(const std::string& cmd);

private:
    std::atomic<bool> isStreaming;
    std::atomic<bool> isPaused;

    std::mutex mtx;
    std::condition_variable condition; // condition variable for streaming state controll
};

#endif //RTSP_MEDIASTREAMHANDLER_H
