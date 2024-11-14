#ifndef RTSP_UDPHANDLER_H
#define RTSP_UDPHANDLER_H

#include "Protos.h"

#include <string>
#include <arpa/inet.h>

class UDPHandler {
public:
    UDPHandler();
    ~UDPHandler();

    bool CreateUDPSocket(int port1, int port2);

    void SendRTPPacket(unsigned char* rtpPacket, size_t packetSize);

    void SendSenderReport(Protos::SenderReport* senderReport, size_t srSize);

    void SetUDPSocket(int port1, int port2);

    int& GetRTPSocket();
    int& GetRTCPSocket();

    sockaddr_in& GetRTPAddr();
    sockaddr_in& GetRTCPAddr();


private:
    int rtpSocket;
    int rtcpSocket;

    sockaddr_in rtpAddr;
    sockaddr_in rtcpAddr;
};

#endif //RTSP_UDPHANDLER_H