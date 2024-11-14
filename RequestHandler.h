#ifndef RTSP_REQUESTHANDLER_H
#define RTSP_REQUESTHANDLER_H

#include "TCPHandler.h"
#include "UDPHandler.h"
#include "ClientSession.h"
#include "MediaStreamHandler.h"

#include <string>

class RequestHandler {
public:
    RequestHandler(MediaStreamHandler& mediaStreamHandler);

    void HandleRequest(int clientSocket, ClientSession* session);

private:
    MediaStreamHandler& mediaStreamHandler;
    
    bool isAlive;

    std::string ParseMethod(const std::string& request);

    int ParseCSeq(const std::string& request);

    // std::pair<int, int> ParsePorts(const std::string& request);

    bool ParseAccept(const std::string& request);

    void HandleOptionsRequest(int clientSocket, int cseq);

    void HandleDescribeRequest(int clientSocket, int cseq, ClientSession* session, const std::string& request);

    void HandleSetupRequest(int clientSocket, int cseq, ClientSession* session, const std::string& request);

    void HandlePlayRequest(int clientSocket, int cseq, ClientSession* session);

    void HandlePauseRequest(int clientSocket, int cseq, ClientSession* session);

    void HandleTeardownRequest(int clientSocket, int cseq, ClientSession* session);
};

#endif //RTSP_REQUESTHANDLER_H