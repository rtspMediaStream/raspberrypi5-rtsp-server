#ifndef GLOBAL_H
#define GLOBAL_H
#include <string.h>

enum ServerStreamType{
    Audio,
    Video,
};

static ServerStreamType g_serverStreamType = ServerStreamType::Audio;
static std::string g_inputFile = "";

#endif //GLOBAL_H
