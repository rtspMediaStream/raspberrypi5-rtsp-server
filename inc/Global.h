/**
 * @file Global.h
 * @brief RTSP Server의 전역변수 및 전역함수 헤더
 * @details RTSP Server에서 사용하는 포트번호 및 전역함수 를 관리합니다.
 * 
 * @organization rtspMediaStream
 * @repository https://github.com/rtspMediaStream/raspberrypi5-rtsp-server
 * 
 * Copyright (c) 2024 rtspMediaStream
 * This project is licensed under the MIT License - see the LICENSE file for details
 */

#ifndef GLOBAL_H
#define GLOBAL_H
#include <string>
#include <utility>
#include <cstdint>

/**
 * @brief 서버의 기본 포트로 8554를 사용합니다.
 */
const int g_serverRtpPort = 8554;

/**
 * @brief 현재 시간을 NTP(Network Time Protocol)타임스탬프 형식으로 계산합니다.
 * 상위 32비트: 1900년 1월 1일(UTC)부터 경과한 초 단위 시간.
 * 하위 32비트: 초의 소수점 이하 부분.
 * @return uint64_t NTP 타임스탬프 형식의 현재 시간
 */
uint64_t GetTime();

/**
 * @brief 주어진 비트 수에 따라 랜덤 숫자를 생성합니다.
 *
 * 이 함수는 입력으로 받은 비트 수(n)에 따라 16비트 또는 32비트 범위의 랜덤 숫자를 생성하여 반환합니다. 
 * 랜덤 시드 생성기를 사용하여 매 호출마다 새로운 랜덤 값을 생성합니다.
 *
 * @param n 생성할 랜덤 숫자의 비트 수. 16 또는 32만 지원합니다.
 * @return uint32_t 생성된 랜덤 숫자.
 *         - n이 32일 경우: 0부터 2^32-1(0xFFFFFFFF) 범위의 랜덤 숫자.
 *         - n이 16일 경우: 0부터 65535(2^16-1) 범위의 랜덤 숫자.
 *         - n이 16 또는 32가 아닐 경우: 0을 반환.
 *
 * @note
 * - `std::random_device`는 랜덤 시드 값을 생성하는 데 사용됩니다.
 * - `std::mt19937`는 Mersenne Twister 알고리즘을 기반으로 한 난수 생성기입니다.
 * - `std::uniform_int_distribution`은 지정된 범위 내에서 균등 분포를 가진 난수를 생성합니다.
 */
uint32_t GetRanNum(int n);

/**
 * @brief 현재 서버의 IP 주소를 가져옵니다.
 *
 * 이 함수는 시스템의 호스트 이름을 기반으로 서버의 IP 주소를 조회하여 반환합니다.
 * 반환된 IP 주소는 IPv4 형식의 문자열로 제공됩니다.
 *
 * @return std::string 서버의 IP 주소. 
 *         - 성공 시: IPv4 형식의 IP 주소 문자열.
 *         - 실패 시: 빈 문자열 ("").
 *
 * @note
 * - `gethostname` 함수는 시스템의 호스트 이름을 가져옵니다.
 * - `gethostbyname` 함수는 호스트 이름을 기반으로 IP 주소 정보를 조회합니다.
 * - `inet_ntoa` 함수는 이진 형식의 IP 주소를 사람이 읽을 수 있는 문자열 형식으로 변환합니다.
 *
 * @warning 
 * - 이 함수는 IPv6 주소를 지원하지 않습니다. IPv4 주소만 반환합니다.
 * - `gethostbyname`는 오래된 함수로, 최신 시스템에서는 `getaddrinfo` 사용을 권장합니다.
 * - 함수 호출 중 오류가 발생하면 `perror`를 통해 에러 메시지가 출력됩니다.
 */
std::string GetServerIP();

#endif //GLOBAL_H
