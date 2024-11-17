#ifndef RTSP_MEDIASTREAMHANDLER_H
#define RTSP_MEDIASTREAMHANDLER_H

#include <atomic>
#include <alsa/asoundlib.h>
#include <condition_variable>


enum MediaStreamState{
    eMediaStream_Init,
    eMediaStream_Play,
    eMediaStream_Pause,
    eMediaStream_Teardown,
};

class MediaStreamHandler {
public:
    UDPHandler* udpHandler;

    MediaStreamHandler();

    void HandleMediaStream();

    unsigned char LinearToUlaw(int sample);
    void InitAlsa(snd_pcm_t*& pcmHandle, snd_pcm_hw_params_t*& params, int& rc, unsigned int& sampleRate, int& dir);
    int CaptureAudio(snd_pcm_t*& pcmHandle, short*& buffer, int& frames, int& rc, unsigned char*& payload);

    void SetCmd(const std::string& cmd);

private:
    MediaStreamState streamState;
    std::mutex streamMutex;
    std::condition_variable condition; // condition variable for streaming state controll
};

#endif //RTSP_MEDIASTREAMHANDLER_H
