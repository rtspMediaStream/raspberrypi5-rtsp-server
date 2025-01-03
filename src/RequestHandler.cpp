/**
 * @file RequestHandler.cpp
 * @brief RequestHandler 클래스의 구현부
 * @details RTSP 프로토콜의 각 메서드 처리 및 응답 생성 구현
 * 
 * Copyright (c) 2024 rtspMediaStream
 * This project is licensed under the MIT License - see the LICENSE file for details
 */

#include "RequestHandler.h"
#include "TCPHandler.h"
#include "ClientSession.h"
#include "MediaStreamHandler.h"
#include "UDPHandler.h"
#include "Global.h"

#include <iostream>
#include <string>
#include <sstream>
#include <thread>

RequestHandler::RequestHandler(ClientSession* session) : session(session){};

/**
 * @details 새로운 스레드를 생성하고 detach하여 백그라운드에서 요청을 처리
 */
void RequestHandler::StartThread() {
    std::thread handlerThread(*this);
    handlerThread.detach();
};

/**
 * @details RTSP 요청 처리 메인 루프:
 *          1. 요청 수신
 *          2. CSeq 및 메서드 파싱
 *          3. 메서드별 적절한 핸들러 호출
 */
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

/**
 * @details RTSP 요청의 첫 줄에서 메서드 이름을 추출
 */
std::string RequestHandler::ParseMethod(const std::string& request) {
    std::istringstream requestStream(request);
    std::string method;
    requestStream >> method;
    return method;
}

/**
 * @details 요청 헤더에서 "CSeq" 필드를 찾아 값을 파싱
 */
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

/**
 * @details Transport 헤더에서 "client_port" 필드를 파싱하여 RTP/RTCP 포트 쌍 추출
 */
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

/**
 * @details Accept 헤더에서 "application/sdp" 지원 여부 확인
 */
bool RequestHandler::ParseAccept(const std::string& request) {
    std::istringstream requestStream(request);
    std::string line;
    while (getline(requestStream, line))
        if (line.find("application/sdp") != std::string::npos)
            return true;
    return false;
}

/**
 * @details 지원하는 RTSP 메서드 목록을 포함한 응답 생성
 */
void RequestHandler::HandleOptionsRequest(const int cseq) {
    std::string response = "RTSP/1.0 200 OK\r\n"
                           "CSeq: " + std::to_string(cseq) + "\r\n"
                           "Public: DESCRIBE, SETUP, TEARDOWN, PLAY, PAUSE\r\n"
                           "\r\n";
    TCPHandler::GetInstance().SendRTSPResponse(session->GetTCPSocket(), response);
}

/**
 * @details 미디어 스트림 정보를 포함한 SDP 응답 생성:
 *          - 비디오(H264)나 오디오(Opus) 스트림 정보
 *          - 세션 ID, 버전, IP 주소 등의 정보 포함
 */
void RequestHandler::HandleDescribeRequest(const std::string& request, const int cseq) {
    std::string ip = GetServerIP();
    std::string sdp = "";
    std::string response = "";

    if (ParseAccept(request)) {
        response = "RTSP/1.0 200 OK\r\n";
        if(RTSPServer::getInstance().getProtocol() == Protocol::PROTO_OPUS) {
            sdp = "v=0\r\n"
                "o=- " + std::to_string(session->GetID()) + " " + std::to_string(session->GetVersion()) + " IN IP4 " + ip + "\r\n"
                "s=Opus Stream\r\n"
                "c=IN IP4 " + ip + "\r\n"
                "t=0 0\r\n"
                "m=audio " + std::to_string(session->GetRTPPort()) + " RTP/AVP 111\r\n"  // Payload type for Opus
                "a=rtpmap:111 opus/48000/2\r\n";  // Opus codec details
        }else if(RTSPServer::getInstance().getProtocol() == Protocol::PROTO_H264) {
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
        }
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

/**
 * @details 스트리밍을 위한 초기 설정 처리:
 *          1. RTP/RTCP 포트 설정
 *          2. UDP 소켓 생성
 *          3. 미디어 스트림 핸들러 초기화
 *          4. 스트리밍 스레드 시작
 */
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

/**
 * @details 미디어 스트림 재생 명령 처리
 */
void RequestHandler::HandlePlayRequest(int cseq) {
    std::string response = "RTSP/1.0 200 OK\r\n"
                           "CSeq: " + std::to_string(cseq) + "\r\n"
                           "Session: " + std::to_string(session->GetID())
                           + "\r\n"
                             "\r\n";
    TCPHandler::GetInstance().SendRTSPResponse(session->GetTCPSocket(), response);

    mediaStreamHandler->SetCmd("PLAY");
}

/**
 * @details 미디어 스트림 일시 정지 명령 처리
 */
void RequestHandler::HandlePauseRequest(int cseq) {
    std::string response = "RTSP/1.0 200 OK\r\n"
                           "CSeq: " + std::to_string(cseq) + "\r\n"
                           "Session: " + std::to_string(session->GetID())
                           + "\r\n"
                             "\r\n";

    TCPHandler::GetInstance().SendRTSPResponse(session->GetTCPSocket(), response);

    mediaStreamHandler->SetCmd("PAUSE");
}

/**
 * @details 세션 종료 및 리소스 정리 처리
 */
void RequestHandler::HandleTeardownRequest(int cseq) {
    std::string response = "RTSP/1.0 200 OK\r\n"
                           "CSeq: " + std::to_string(cseq) + "\r\n"
                           "Session: " + std::to_string(session->GetID())
                           + "\r\n"
                             "\r\n";

    TCPHandler::GetInstance().SendRTSPResponse(session->GetTCPSocket(), response);

    mediaStreamHandler->SetCmd("TEARDOWN");
}
