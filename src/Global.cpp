#include "Global.h"

#include <chrono>
#include <random>
#include <netdb.h>
#include <iomanip>
#include <unistd.h>
#include <arpa/inet.h>

/**
 * @brief 현재 시간을 NTP(Network Time Protocol)타임스탬프 형식으로 계산합니다.
 * 상위 32비트: 1900년 1월 1일(UTC)부터 경과한 초 단위 시간.
 * 하위 32비트: 초의 소수점 이하 부분.
 * @return uint64_t NTP 타임스탬프 형식의 현재 시간
 */
uint64_t GetTime() {
    auto now = std::chrono::system_clock::now();
    auto msSinceEpoch = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
    uint32_t ntpSeconds = (msSinceEpoch / 1000) + 2208988800U; // 1970 to 1900
    uint32_t ntpFraction = (msSinceEpoch % 1000) * 4294967;
    uint64_t ntpTime = ((uint64_t)ntpSeconds << 32) | ntpFraction;
    return ntpTime;
}

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
uint32_t GetRanNum(int n) {
    std::random_device rd; // make random seed
    std::mt19937 generator(rd()); // init random number generator
    if (n == 32) {
        // generate random number between 0 and 2^32-1
        std::uniform_int_distribution<uint32_t> distribution(1, 0xFFFFFFFF);
        return distribution(generator);
    } else if (n == 16) {
        std::uniform_int_distribution<uint32_t> distribution(1, 65535);
        return distribution(generator);
    }
    return 0;
}

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
std::string GetServerIP() {
    char hostbuffer[256];
    char *IPbuffer;
    struct hostent *host_entry;
    int hostname;

    // Get hostname
    hostname = gethostname(hostbuffer, sizeof(hostbuffer));
    if (hostname == -1) {
        perror("gethostname");
        return std::string("");
    }

    // Get host information
    host_entry = gethostbyname(hostbuffer);
    if (host_entry == nullptr) {
        perror("gethostbyname");
        return std::string("");
    }

    // Convert host's binary address into text
    IPbuffer = inet_ntoa(*((struct in_addr *) host_entry->h_addr_list[0]));
    return IPbuffer;
}