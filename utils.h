#ifndef RTSP_UTILS_H
#define RTSP_UTILS_H

#include <string>
#include <utility>

class utils {
public:
    static uint64_t getTime();
    static uint32_t getRanNum(int n);
    static char* getIP();
};

#endif //RTSP_UTILS_H
