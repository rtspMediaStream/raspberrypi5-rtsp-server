#include "DataCapture.h"
#include <alsa/asoundlib.h>
#include <queue>
#include <mutex>

/**
 * @class AudioCapture
 * @brief Audio Stream Data를 캡처하는 클래스
 * @details OpusEncoder를 이용해서 Audio Stream을 캡처합니다.
 */
class AudioCapture : public DataCapture{
public:
    /**
     * @brief AudioCapture 생성자
     * @details PCM 옵션을 설정하고 PCM handle을 획득합니다.
     */
    AudioCapture();
    /**
     * @brief pcm에서 음성 데이터를 읽습니다
     * @param buffer 음성 데이터의 시작 포인터
     * @param frames 음성 데이터의 크기
     * @details PCM handle에서 frames 크기만큼 데이터를 읽어와 buffer에 복사합니다.
     * @return PCM errorCode
     */
    int read(short *buffer, int frames);
    /**
     * @brief AudioCapture 소멸자
     * @details PCM handle을 반환합니다.
     */
    ~AudioCapture();
private:
    snd_pcm_t* pcm_handle;
    snd_pcm_hw_params_t* params;
    unsigned int sample_rate;
    int dir;
};