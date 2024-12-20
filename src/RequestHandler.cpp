#include "utils.h"
#include "RequestHandler.h"
#include "TCPHandler.h"
#include "ClientSession.h"
#include "MediaStreamHandler.h"
#include "UDPHandler.h"
#include "global.h"

#include <iostream>
#include <string>
#include <sstream>
#include <thread>

RequestHandler::RequestHandler(ClientSession* session) : session(session){};

void RequestHandler::StartThread() {
    std::thread handlerThread(*this);
    handlerThread.detach();
};

void RequestHandler::HandleRequest() {
    while (true) {
        std::cout << "recv wait id :" << session->GetTCPSocket() << std::endl;
        
        std::string request = TCPHandler::GetInstance().ReceiveRTSPRequest(session->GetTCPSocket());
        if (request.empty()) {
            std::cerr << "Invalid RTSP request received." << std::endl;
            return;
        }

        int cseq = ParseCSeq(request);
        if (cseq == -1) {
            std::cerr << "CSeq parsing failed." << std::endl;
            return;
        }

        std::string method = ParseMethod(request);
        if (method == "OPTIONS") {
            HandleOptionsRequest(cseq);
        } else if (method == "DESCRIBE") {
            HandleDescribeRequest(request, cseq);
        } else if (method == "SETUP") {
            HandleSetupRequest(request, cseq);
        } else if (method == "PLAY") {
            HandlePlayRequest(cseq);
        } else if (method == "PAUSE") {
            HandlePauseRequest(cseq);
        } else if (method == "TEARDOWN") {
            HandleTeardownRequest(cseq);
            break;
        } else {
            std::cerr << "Unsupported RTSP method: " << method << std::endl;
        }
    }
}

std::string RequestHandler::ParseMethod(const std::string& request) {
    std::istringstream requestStream(request);
    std::string method;
    requestStream >> method;
    return method;
}

int RequestHandler::ParseCSeq(const std::string& request) {
    std::istringstream requestStream(request);
    std::string line;
    while (getline(requestStream, line)) {
        if (line.find("CSeq") != std::string::npos) {
            std::istringstream lineStream(line);
            std::string label;
            int cseq;
            lineStream >> label >> cseq;
            return cseq;
        }
    }
    return -1; // CSeq not found
}

std::pair<int, int> RequestHandler::ParsePorts(const std::string& request) {
    std::istringstream requestStream(request);
    std::string line;
    while (getline(requestStream, line)) {
        if (line.find("client_port=") != std::string::npos) {
            std::istringstream lineStream(line);
            std::string label;

            while (getline(lineStream, label, '/')) {
                std::string portRange;
                getline(lineStream, portRange);
                size_t eqPos = portRange.find('=') + 1;
                size_t dashPos = portRange.find('-');

                if (dashPos != std::string::npos) {
                    
                    int rtpPort = stoi(portRange.substr(eqPos, dashPos - eqPos));
                    int rtcpPort = stoi(portRange.substr(dashPos + 1));
		            return {rtpPort, rtcpPort};
                }
            }
        }
    }
    return {-1, -1};
}

bool RequestHandler::ParseAccept(const std::string& request) {
    std::istringstream requestStream(request);
    std::string line;
    while (getline(requestStream, line))
        if (line.find("application/sdp") != std::string::npos)
            return true;
    return false;
}

void RequestHandler::HandleOptionsRequest(const int cseq) {
    std::string response = "RTSP/1.0 200 OK\r\n"
                           "CSeq: " + std::to_string(cseq) + "\r\n"
                           "Public: DESCRIBE, SETUP, TEARDOWN, PLAY, PAUSE\r\n"
                           "\r\n";
    TCPHandler::GetInstance().SendRTSPResponse(session->GetTCPSocket(), response);
}

void RequestHandler::HandleDescribeRequest(const std::string& request, const int cseq) {
    std::string ip = utils::GetServerIP();
    std::string sdp = "";
    std::string response = "";

    if (ParseAccept(request)) {
        response = "RTSP/1.0 200 OK\r\n";

        //TODO SDP 통합 필요
        // if(ServerStream::getInstance().type == ServerStreamType::Audio) {
        //     sdp = "v=0\r\n"
        //       "o=- " + std::to_string(session->GetID()) + " " + std::to_string(session->GetVersion()) +
        //       " IN IP4 " + ip + "\r\n"
        //       "s=Opus Stream\r\n"
        //       "c=IN IP4 " + ip + "\r\n"
        //       "t=0 0\r\n"
        //       "m=audio " + std::to_string(session->GetRTPPort()) + " RTP/AVP 111\r\n"  // Payload type for Opus
        //       "a=rtpmap:111 opus/48000/2\r\n";  // Opus codec details
        // }else if(ServerStream::getInstance().type == ServerStreamType::Video) {
            sdp = "v=0\r\n"
                "o=- 0 0 IN IP4 " + ip + "\r\n"
                "s=H264 Video Stream\r\n"
                "c=IN IP4 " + ip + "\r\n"
                "t=0 0\r\n"
                "a=tool:libavformat LIBAVFORMAT_VERSION\r\n"
                "m=video " + std::to_string(session->GetRTPPort()) + " RTP/AVP 96\r\n"
                "b=AS:40\r\n"
                "a=rtpmap:96 H264/90000\r\n"
                "a=fmtp:96 packetization-mode=1\r\n";
        //}
    } else {
        response = "RTSP/1.0 406 Not Acceptable\r\n";
    }

    response += "CSeq: " + std::to_string(cseq) + "\r\n"
                "Content-Base: rtsp://" + ip + ":" + std::to_string(g_serverRtpPort) +"/\r\n"
                "Content-Type: application/sdp\r\n"
                "Content-Length: " + std::to_string(sdp.size()) + "\r\n"
                "\r\n" + sdp;

    TCPHandler::GetInstance().SendRTSPResponse(session->GetTCPSocket(), response);
}


void RequestHandler::HandleSetupRequest(const std::string& request, const int cseq) {
    auto ports = ParsePorts(request);
    if (ports.first < 0 || ports.second < 0) {
        std::cerr << "not found IP or Port in SETUP" << std::endl;
        return;
    }
    session->SetRTPPort(ports.first);
    session->SetRTCPPort(ports.second);

    std::string response = "RTSP/1.0 200 OK\r\n"
                           "CSeq: " + std::to_string(cseq) + "\r\n"
                           "Transport: RTP/AVP;unicast;client_port="
                           + std::to_string(session->GetRTPPort()) + "-"
                           + std::to_string(session->GetRTCPPort()) + "\r\n"
                           "Session: " + std::to_string(session->GetID())
                           + "\r\n"
                             "\r\n";
    TCPHandler::GetInstance().SendRTSPResponse(session->GetTCPSocket(), response);

    RTSPServer::getInstance().onInitEvent();  //TODO : play function when occur init event

    mediaStreamHandler = new MediaStreamHandler();
    mediaStreamHandler->udpHandler = new UDPHandler(session);
    mediaStreamHandler->udpHandler->CreateUDPSocket();
    std::thread mediaStreamThread(&MediaStreamHandler::HandleMediaStream, mediaStreamHandler);

    mediaStreamThread.detach();
}

void RequestHandler::HandlePlayRequest(int cseq) {
    std::string response = "RTSP/1.0 200 OK\r\n"
                           "CSeq: " + std::to_string(cseq) + "\r\n"
                           "Session: " + std::to_string(session->GetID())
                           + "\r\n"
                             "\r\n";
    TCPHandler::GetInstance().SendRTSPResponse(session->GetTCPSocket(), response);

    mediaStreamHandler->SetCmd("PLAY");
}

void RequestHandler::HandlePauseRequest(int cseq) {
    std::string response = "RTSP/1.0 200 OK\r\n"
                           "CSeq: " + std::to_string(cseq) + "\r\n"
                           "Session: " + std::to_string(session->GetID())
                           + "\r\n"
                             "\r\n";

    TCPHandler::GetInstance().SendRTSPResponse(session->GetTCPSocket(), response);

    mediaStreamHandler->SetCmd("PAUSE");
}

void RequestHandler::HandleTeardownRequest(int cseq) {
    std::string response = "RTSP/1.0 200 OK\r\n"
                           "CSeq: " + std::to_string(cseq) + "\r\n"
                           "Session: " + std::to_string(session->GetID())
                           + "\r\n"
                             "\r\n";

    TCPHandler::GetInstance().SendRTSPResponse(session->GetTCPSocket(), response);

    mediaStreamHandler->SetCmd("TEARDOWN");
}
