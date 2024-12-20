project(AudioServer)
cmake_minimum_required(VERSION 3.10)

# Add include directories
include_directories(
    ${CMAKE_SOURCE_DIR}/inc
    /usr/include/aarch64-linux-gnu/
)

# Use pkg-config to find required libraries
find_package(PkgConfig REQUIRED)
pkg_check_modules(PKG_LIBS REQUIRED opus alsa libevent_pthreads)


# C++ 표준 설정
set(CMAKE_CXX_STANDARD 17)

# 소스 파일 및 헤더 파일 지정
set(SOURCES
    main.cpp
    AudioCapture.cpp
)

set(HEADERS
    AudioCapture.h
)

# 타겟 생성
add_executable(AudioServer ${SOURCES})

# include 경로 추가
target_include_directories(AudioServer PRIVATE
    ${CMAKE_SOURCE_DIR}/../../inc
)

# 필요한 라이브러리 링크 (필요시 추가)
target_link_libraries(AudioServer -lrtspserver -lasound)
