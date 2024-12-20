#ifndef GLOBAL_H
#define GLOBAL_H
#include <string.h>


const int g_serverRtpPort = 8554;

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

#endif //GLOBAL_H
