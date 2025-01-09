/**
* @file AudioCapture.h
* @brief 오디오 스트림 캡처 및 관리 클래스 헤더
* @details ALSA를 사용하여 PCM 오디오 데이터를 캡처하고 관리하는 클래스
*          - PCM 디바이스 설정 및 관리
*          - 오디오 스트림 데이터 캡처
*          - Opus 인코딩을 위한 오디오 데이터 처리
* 
* @organization rtspMediaStream
* @repository https://github.com/rtspMediaStream/raspberrypi5-rtsp-server
* 
* Copyright (c) 2024 rtspMediaStream
* This project is licensed under the MIT License - see the LICENSE file for details
*/

#include "DataCapture.h"
#include <alsa/asoundlib.h>
#include <queue>
#include <mutex>

/**
 * @class AudioCapture
 * @brief Audio Stream Data를 캡처하는 클래스
 * @details PCM 오디오 데이터를 캡처하고 Opus 인코딩을 위한 처리를 수행하는 클래스로,
 *          DataCapture 클래스를 상속받아 오디오 스트림 관리 기능을 구현합니다.
 */
class AudioCapture : public DataCapture{
public:
    /**
     * @brief AudioCapture 생성자
     *   - ALSA PCM 디바이스 초기화
     *   - PCM 하드웨어 파라미터 설정
     *   - 오디오 캡처를 위한 PCM 핸들 획득
     */
    AudioCapture();
   /**
    * @brief PCM 디바이스에서 오디오 데이터를 읽는 함수
    * @param buffer 오디오 데이터를 저장할 버퍼 포인터
    * @param frames 읽을 프레임 수
    * @return int 성공 시 읽은 프레임 수, 실패 시 ALSA 에러 코드
    * @details PCM 핸들을 통해 지정된 프레임 수만큼 오디오 데이터를 읽어 버퍼에 저장
    */
    int read(short *buffer, int frames);
    /**
     * @brief AudioCapture 소멸자
     * @details 할당된 PCM handle을 반환
     */
    ~AudioCapture();
private:
    snd_pcm_t* pcm_handle;       ///< PCM 디바이스 핸들
    snd_pcm_hw_params_t* params; ///< PCM 하드웨어 파라미터
    unsigned int sample_rate;    ///< 샘플링 레이트
    int dir;                     ///< ALSA PCM 설정 방향
};