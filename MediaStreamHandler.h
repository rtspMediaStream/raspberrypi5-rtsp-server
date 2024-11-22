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

    uint8_t pcm_to_ulaw(int16_t pcm_val);

    void SetCmd(const std::string& cmd);

private:
    MediaStreamState streamState;
    std::mutex streamMutex;
    std::condition_variable condition; // condition variable for streaming state controll
};

#endif //RTSP_MEDIASTREAMHANDLER_H
