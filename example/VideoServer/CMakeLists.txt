project(VideoServer)
cmake_minimum_required(VERSION 3.10)

# C++ 표준 설정
set(CMAKE_CXX_STANDARD 17)

# 소스 파일 및 헤더 파일 지정
set(SOURCES
    main.cpp
)

set(HEADERS
)

# 타겟 생성
add_executable(VideoServer ${SOURCES})

# include 경로 추가
target_include_directories(VideoServer PRIVATE
    ${CMAKE_SOURCE_DIR}/../../inc
)

# 필요한 라이브러리 링크 (필요시 추가)
target_link_libraries(VideoServer -lrtspserver)
