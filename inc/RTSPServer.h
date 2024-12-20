#ifndef __RTSPSERVER_H__
#define __RTSPSERVER_H__
#include <functional>
class FFmpegEncoder;

enum Protocol {
    PROTO_OPUS = 111,
    PROTO_H264 = 96,
};


class RTSPServer {
private:
    bool isPrivilegedPort(int port);
    bool isRunningAsRoot();
    Protocol protocol;

public:
    static RTSPServer& getInstance() {
        static RTSPServer instance;
        return instance;
    }
    RTSPServer();
    ~RTSPServer();
    int startServerThread();
    void setProtocol(Protocol _ptorocol) { protocol = _ptorocol; };
    std::function<void()> onInitEvent;
};

#endif // __RTSPSERVER_H__