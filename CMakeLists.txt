# Minimum CMake version required
cmake_minimum_required(VERSION 3.10)

# Project name and version
project(RTSPServer VERSION 1.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Set the output name for the shared library
set(TARGET_NAME rtspserver)

# Enable position-independent code (PIC) for shared library
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

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
pkg_check_modules(PKG_LIBS REQUIRED opus alsa opencv4 libcamera libevent_pthreads libavcodec libavformat libswscale)

# Add include directories from pkg-config
include_directories(${PKG_LIBS_INCLUDE_DIRS})

# Add linker flags from pkg-config
link_directories(${PKG_LIBS_LIBRARY_DIRS})

# Collect source files
file(GLOB_RECURSE SOURCES ${CMAKE_SOURCE_DIR}/src/*.cpp)

# Exclude main.cpp (if it exists)
list(FILTER SOURCES EXCLUDE REGEX "main\\.cpp$")

# Define the shared library target
add_library(${TARGET_NAME} SHARED ${SOURCES})

# Link libraries
target_link_libraries(${TARGET_NAME} ${PKG_LIBS_LIBRARIES} avcodec avformat avutil swscale)

# Installation rules (optional)
install(TARGETS ${TARGET_NAME} DESTINATION /usr/local/lib)
install(DIRECTORY ${CMAKE_SOURCE_DIR}/inc/ DESTINATION /usr/local/include)

