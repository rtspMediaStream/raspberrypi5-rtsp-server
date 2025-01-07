#ifndef FFMPEG_ENCODER_H
#define FFMPEG_ENCODER_H
#include <opencv2/core.hpp>
class AVStream;

/**
 * @class FFmpegEncoder
 * @brief FFmpeg을 이용해 이미지를 변환하는 클래스
 * @details FFmpeg library를 이용해 이미지를 변환합니다.
 */
class FFmpegEncoder {
public:
    /**
     * @brief FFmpegEncoder 생성자
     * @details AVCodec을 설정하고 ffmpeg library를 이용해 라즈베리파이 카메라모듈과 연결합니다.
     */
    FFmpegEncoder(const std::string& filename, int width, int height, double fps);
    /**
     * @brief FFmpegEncoder 소멸자
     * @details ffmpeg에 연결된 카메라모듈을 반환합니다.
     */
    ~FFmpegEncoder();
    /**
     * @brief 카메라 모듈에서 프레임을 읽어옵니다.
     * @param frame YUVformat형태의 프레임
     * @param fps 카메라 모듈에 설정된 frame per seconds
     * @details 카메라 모듈에서 YUVformat의 프레임을 읽어와 DataCapture로 프레임을 처리합니다.
     */
    void encode(const cv::Mat& frame, double fps);

private:
    /**
     * @brief FFmpeg library로 카메라를 설정합니다.
     * @details AVCodec을 설정하고 ffmpeg library를 이용해 라즈베리파이 카메라모듈과 연결합니다.
     */
    void initFFmpeg(const std::string& filename, double fps);
    /**
     * @brief FFmpeg library로 카메라를 설정합니다.
     * @details AVCodec을 설정하고 ffmpeg library를 이용해 라즈베리파이 카메라모듈과 연결합니다.
     */
    void releaseFFmpeg();

    // FFmpeg 관련 변수 선언
    struct AVCodecContext *codec_ctx = nullptr;
    struct AVFormatContext *fmt_ctx = nullptr;
    struct AVPacket *packet = nullptr;
    struct AVFrame *frame = nullptr;
    int frame_index = 0;
    AVStream *stream = nullptr;

    int width;
    int height;
};

#endif // FFMPEG_ENCODER_H