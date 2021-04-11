cmake_minimum_required(VERSION 3.10)
project(Streamer VERSION 1.0)

set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")

set (BOOST_LIBS
    system
    program_options
    log
    thread
    date_time
    regex
    serialization
    atomic
    chrono
)

# boost options
set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_MULTITHREADED ON)

find_package(Boost REQUIRED COMPONENTS ${BOOST_LIBS})

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

# message("ffmpeg libs = ${FFMPEG_LINK_LIBRARIES}")
# message("ffmpeg include dirs = ${FFMPEG_INCLUDE_DIRS}")
# message("ffmpeg cflags = ${FFMPEG_CFLAGS}")

add_subdirectory(utils-lib)
add_subdirectory(network-lib)
add_subdirectory(core-lib)

add_subdirectory(stream-sender)
add_subdirectory(tests)