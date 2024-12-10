#ifndef RTSP_REQUESTHANDLER_H
#define RTSP_REQUESTHANDLER_H

#include <memory>
#include <string>
class ClientSession;
class MediaStreamHandler;


class RequestHandler {
public:
    RequestHandler(ClientSession* session);

    void operator()() { HandleRequest(); }

    void HandleRequest();

    void StartThread();

private:
    std::shared_ptr<ClientSession> session;

    MediaStreamHandler *mediaStreamHandler;

    std::string ParseMethod(const std::string& request);

    int ParseCSeq(const std::string& request);

    std::pair<int, int> ParsePorts(const std::string& request);

    bool ParseAccept(const std::string& request);

    void HandleOptionsRequest(const int cseq);

    void HandleDescribeRequest(const std::string& request, const int cseq);

    void HandleSetupRequest(const std::string& request, const int cseq);

    void HandlePlayRequest(int cseq);

    void HandlePauseRequest(int cseq);

    void HandleTeardownRequest(int cseq);
};

#endif //RTSP_REQUESTHANDLER_H