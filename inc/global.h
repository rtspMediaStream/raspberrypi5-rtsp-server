#ifndef GLOBAL_H
#define GLOBAL_H
#include <string.h>


const int g_serverRtpPort = 554;

enum ServerStreamType{
    Audio = 1,
    Video,
    Camera,
};

class ServerStream{
    public:
    ServerStreamType type;
    static ServerStream& getInstance(){
        static ServerStream instance;
        return instance;
    }
};

static std::string g_inputFile = "example/dragon.h264";

#endif //GLOBAL_H
