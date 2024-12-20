#include "RTSPServer.h"
#include <unistd.h>
#include <iostream>
#include "OpusEncoder.h"
#include "AudioCapture.h"

#include <thread>
#include "H264Encoder.h"
#include "DataCapture.h"
#include "utils.h"

// Read Audio from Opus
void LoadOpus()
{
    std::thread([]()->void {
                     short pcmBuffer[OPUS_FRAME_SIZE * OPUS_CHANNELS];
                     OpusEncoder opusEncoder;
                     DataCaptureFrame newFrame;
                     newFrame.timestamp = (unsigned int)utils::GetRanNum(16);
                     AudioCapture audioCapture;
                     while(1){
                        newFrame.dataPtr = new unsigned char[MAX_PACKET_SIZE];
                         
                        int rc = audioCapture.read(pcmBuffer, OPUS_FRAME_SIZE);
                        if (rc != OPUS_FRAME_SIZE)
                        {
                            std::cout << "occur audio packet skip." << std::endl;
                            continue;
                        }
                       
                        int bufferSize = opusEncoder.encode(pcmBuffer, OPUS_FRAME_SIZE, newFrame.dataPtr);
                        newFrame.size = bufferSize;
                        newFrame.timestamp += OPUS_FRAME_SIZE;
                        if (bufferSize <= 0)
                        {
                            std::cerr << "Opus encoding error: " << bufferSize << std::endl;
                            delete[] newFrame.dataPtr;
                            continue;
                        }

                        AudioCapture::getInstance().pushFrame(newFrame);
                     }
                     return ;
                 } ).detach();
}

int main(int argc, char *argv[])
{
    RTSPServer::getInstance().setProtocol(Protocol::PROTO_OPUS);
    RTSPServer::getInstance().onInitEvent = LoadOpus;
    RTSPServer::getInstance().startServerThread();

    while (1)
    {
        sleep(1);
    }
    
        
    return 0;
}