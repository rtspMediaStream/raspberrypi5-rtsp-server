#include <queue>
#include <mutex>

class VideoCapture{
    std::queue <std::pair<unsigned char*, int>> imgBuffer;
    std::mutex imgBufferMutex;
public:
    inline int getImgBufferSize() { return imgBuffer.size(); };
    void pushImg(unsigned char* imgPtr, int size);
    std::pair<unsigned char*, int> popImg();
};