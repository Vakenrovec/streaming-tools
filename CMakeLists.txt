cmake_minimum_required(VERSION 3.10)
project(Streamer VERSION 1.0)

set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(PkgConfig REQUIRED)

pkg_check_modules(FFMPEG REQUIRED
    # libavdevice
    # libavfilter
    libavformat
    libavcodec
    # libswresample
    libswscale
    libavutil
)

pkg_check_modules(SDL2 REQUIRED 
    sdl2
)

message("ffmpeg libs = ${FFMPEG_LINK_LIBRARIES}")
message("ffmpeg include dirs = ${FFMPEG_INCLUDE_DIRS}")
message("ffmpeg cflags = ${FFMPEG_CFLAGS}")

add_subdirectory(utils-lib)
add_subdirectory(core-lib)
add_subdirectory(stream-sender)
