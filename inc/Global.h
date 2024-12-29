#ifndef GLOBAL_H
#define GLOBAL_H
#include <string>
#include <utility>
#include <cstdint>


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

uint64_t GetTime();
uint32_t GetRanNum(int n);
std::string GetServerIP();

#endif //GLOBAL_H
