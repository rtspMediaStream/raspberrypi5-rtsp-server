#ifndef __DATACAPTURE_H__
#define __DATACAPTURE_H__
#include <vector>
#include <queue>
#include <mutex>

struct DataCaptureFrame {
    unsigned char *dataPtr;
    unsigned int size;
    unsigned int timestamp;
};

class DataCapture {
public:
    static const int buffer_max_size = 10;
    static DataCapture &getInstance()
    {
        static DataCapture instance;
        return instance;
    }
    inline bool isEmptyBuffer() { return (buffer_size == 0); };
    inline bool isFullBuffer() { return (buffer_size == buffer_max_size); };
    virtual void pushFrame(const DataCaptureFrame& frame);
    virtual DataCaptureFrame popFrame();
protected:
    std::vector <DataCaptureFrame> frameBuffer;
    int head = 0;
    int tail = 0;
    int buffer_size = 0;

    std::mutex bufferMutex;
    DataCapture()
    {
        frameBuffer.resize(buffer_max_size, {nullptr, 0 , 0});
    }

    ~DataCapture()
    {
        for(auto& pair : frameBuffer) {
            delete[] pair.dataPtr;
        }
    }
};

#endif //__DATACAPTURE_H__