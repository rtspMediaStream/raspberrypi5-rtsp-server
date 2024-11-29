#ifndef RTSP_MEDIASTREAMHANDLER_H
#define RTSP_MEDIASTREAMHANDLER_H

#include <atomic>
#include <alsa/asoundlib.h>
#include <condition_variable>

#define MAX_RTP_PAYLOAD_SIZE 1400 // RTP Payload 최대 크기 (일반적으로 MTU - Header 크기)

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
    void SendFragmentedRTPPackets(unsigned char* payload, size_t payloadSize, Protos::RTPHeader& rtpHeader, unsigned short &seqNum);
};

#endif //RTSP_MEDIASTREAMHANDLER_H
