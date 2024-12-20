project(CameraServer)
cmake_minimum_required(VERSION 3.10)

# Add include directories
include_directories(
    ${CMAKE_SOURCE_DIR}/inc
    /usr/include/opencv4
    /usr/include/opencv4/opencv2
    /usr/include/libcamera
    /usr/include/aarch64-linux-gnu/
)

# Use pkg-config to find required libraries
find_package(PkgConfig REQUIRED)
pkg_check_modules(PKG_LIBS REQUIRED opencv4 libcamera libevent_pthreads libavcodec libavformat libswscale libavutil)

# C++ 표준 설정
set(CMAKE_CXX_STANDARD 17)

# 소스 파일 및 헤더 파일 지정
set(SOURCES
    main.cpp
    FFmpegEncoder.cpp
)

set(HEADERS
    FFmpegEncoder.h
)

# 타겟 생성
add_executable(CameraServer ${SOURCES})

# include 경로 추가
target_include_directories(CameraServer PRIVATE
    ${CMAKE_SOURCE_DIR}/../../inc
)

# 필요한 라이브러리 링크 (필요시 추가)
target_link_libraries(CameraServer ${PKG_LIBS_LIBRARIES} -lrtspserver)