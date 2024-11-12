#ifndef RTSP_UDPHANDLER_H
#define RTSP_UDPHANDLER_H

#include "Protos.h"

#include <string>
#include <arpa/inet.h>

class UDPHandler {
public:
    UDPHandler();
    ~UDPHandler();

    bool createUDPSocket(int port1, int port2);

    void sendRTPPacket(unsigned char* rtpPacket, size_t packetSize);

    void sendSenderReport(Protos::SenderReport* senderReport, size_t srSize);

    void setUDPSocket(int port1, int port2);

    int& getRTPSocket();
    int& getRTCPSocket();

    sockaddr_in& getRTPAddr();
    sockaddr_in& getRTCPAddr();


private:
    int rtpSocket;
    int rtcpSocket;

    sockaddr_in rtpAddr;
    sockaddr_in rtcpAddr;
};

#endif //RTSP_UDPHANDLER_H