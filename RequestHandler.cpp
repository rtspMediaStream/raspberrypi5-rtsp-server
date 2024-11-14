#include "utils.h"
#include "RequestHandler.h"

#include <iostream>
#include <string>
#include <sstream>
#include <thread>

#define TCP TCPHandler::GetInstance()

RequestHandler::RequestHandler(const std::shared_ptr<Info>& client)
        : mediaStreamHandler(mediaStreamHandler), client(client) {}

void RequestHandler::HandleRequest() {
    // std::cout << "create Client Session" << std::endl;

    while (true) {
        std::string request = TCP.ReceiveRTSPRequest(client->id);
        if (request.empty()) {
            std::cerr << "Invalid RTSP request received." << std::endl;
            return;
        }

        std::string method = ParseMethod(request);

        int cseq = ParseCSeq(request);
        if (cseq == -1) {
            std::cerr << "CSeq parsing failed." << std::endl;
            return;
        }

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
                size_t dashPos = portRange.find('-');

                if (dashPos != std::string::npos) {
                    int rtpPort = stoi(portRange.substr(0, dashPos));
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

void RequestHandler::HandleOptionsRequest(int cseq) {
    std::string response = "RTSP/1.0 200 OK\r\n"
                           "CSeq: " + std::to_string(cseq) + "\r\n"
                           "Public: DESCRIBE, SETUP, TEARDOWN, PLAY, PAUSE\r\n"
                           "\r\n";
    TCP.SendRTSPResponse(client->tcpSocket, response);
}

void RequestHandler::HandleDescribeRequest(const std::string& request, int cseq) {
    std::string ip = "";
    std::string sdp = "";
    std::string response = "";
    if (ParseAccept(request)) {
        response = "RTSP/1.0 200 OK\r\n";

        ip = utils::GetIP();
        sdp = "v=0\r\n"
              "o=- "
              + std::to_string(client->id)
              + " " + std::to_string(client->version)
              + " IN IP4 " + ip + "\r\n"
              "s=Audio Stream\r\n"
              "c=IN IP4 " + ip + "\r\n"
              "t=0 0\r\n"
              "m=audio " + std::to_string(client->rtpPort)
              + " RTP/AVP 0\r\n"
                "a=rtpmap:0 PCMU/8000\r\n";
    } else
        response = "RTSP/1.0 406 Not Acceptable\r\n";

    response += "CSeq: " + std::to_string(cseq) + "\r\n"
                "Content-Base: rtsp://" + ip + ":8554/\r\n"
                "Content-Type: application/sdp\r\n"
                "Content-Length: " + std::to_string(sdp.size()) + "\r\n"
                "\r\n" + sdp;
    TCP.SendRTSPResponse(client->tcpSocket, response);
}

void RequestHandler::HandleSetupRequest(const std::string& request, int cseq) {
    client->state = "SETUP";

    auto ports = ParsePorts(request);
    if (ports.first < 0 || ports.second < 0) {
        std::cerr << "클라이언트 포트가 없습니다" << std::endl;
        return;
    }

    client->rtpPort = ports.first;
    client->rtcpPort = ports.second;
    // session->SetPort(ports.first, ports.second);

    std::string response = "RTSP/1.0 200 OK\r\n"
                           "CSeq: " + std::to_string(cseq) + "\r\n"
                           "Transport: RTP/AVP;unicast;client_port="
                           + std::to_string(client->rtpPort) + "-"
                           + std::to_string(client->rtcpPort) + "\r\n"
                           "Session: " + std::to_string(client->id)
                           + "\r\n"
                             "\r\n";
    TCP.SendRTSPResponse(client->tcpSocket, response);

    std::thread mediaStreamThread(&MediaStreamHandler::HandleMediaStream, &mediaStreamHandler);
    mediaStreamThread.detach();
}

void RequestHandler::HandlePlayRequest(int cseq) {
    client->state = "PLAY";

    std::string response = "RTSP/1.0 200 OK\r\n"
                           "CSeq: " + std::to_string(cseq) + "\r\n"
                           "Session: " + std::to_string(client->id)
                           + "\r\n"
                             "\r\n";
    TCP.SendRTSPResponse(client->tcpSocket, response);

    mediaStreamHandler.SetCmd("PLAY");
}

void RequestHandler::HandlePauseRequest(int cseq) {
    client->state = "PAUSE";

    std::string response = "RTSP/1.0 200 OK\r\n"
                           "CSeq: " + std::to_string(cseq) + "\r\n"
                           "Session: " + std::to_string(client->id)
                           + "\r\n"
                             "\r\n";

    TCP.SendRTSPResponse(client->tcpSocket, response);

    mediaStreamHandler.SetCmd("PAUSE");
}

void RequestHandler::HandleTeardownRequest(int cseq) {
    client->state = "TEARDOWN";

    std::string response = "RTSP/1.0 200 OK\r\n"
                           "CSeq: " + std::to_string(cseq) + "\r\n"
                           "Session: " + std::to_string(client->id)
                           + "\r\n"
                             "\r\n";

    TCP.SendRTSPResponse(client->tcpSocket, response);

    mediaStreamHandler.SetCmd("TEARDOWN");
}
