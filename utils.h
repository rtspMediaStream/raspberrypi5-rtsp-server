#ifndef RTSP_UTILS_H
#define RTSP_UTILS_H

#include <string>
#include <utility>

class utils {
public:
    static uint64_t GetTime();
    static uint32_t GetRanNum(int n);
    static char* GetIP();
};

#endif //RTSP_UTILS_H
