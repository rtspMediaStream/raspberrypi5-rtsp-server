#include <vector>
#include <queue>
#include <mutex>

class VideoCapture{
public:
    static const int buffer_max_size = 5;

    static VideoCapture& getInstance() {
        static VideoCapture instance;
        return instance;
    }
    inline bool isEmptyBuffer() { return (buffer_size == 0); };
    inline bool isFullBuffer() { return (buffer_size == buffer_max_size); };
    void pushImg(unsigned char* imgPtr, int size);
    std::pair<unsigned char*, int> popImg();
private:
    std::vector <std::pair<unsigned char*, int>> imgBuffer;
    int head = 0;
    int tail = 0;
    int buffer_size = 0;

    std::mutex imgBufferMutex;

    VideoCapture()
    {
        imgBuffer.resize(buffer_max_size, std::make_pair(nullptr, 0)); // 초기화
    }

    ~VideoCapture() {
        for (auto& pair : imgBuffer) {
            delete[] pair.first; // 동적 할당 해제
        }
    }
};