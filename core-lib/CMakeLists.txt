set (PROJECT_NAME core-lib)

set(HEADERS 
    Image.h
    codecs/vp8.h
    devices/WebCamera.h
)

set(SOURCES
    codecs/vp8.cpp
    devices/WebCamera.cpp
)

source_group("Source Files" FILES ${SOURCES})
source_group("Header Files" FILES ${HEADERS})

set(PROJECT_CONTENT ${SOURCES} ${HEADERS})

add_library(${PROJECT_NAME} STATIC ${PROJECT_CONTENT})

target_include_directories(${PROJECT_NAME}
    PRIVATE
        .
        ../utils-lib
        ${FFMPEG_INCLUDE_DIRS}
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        # ${FFMPEG_LIBRARIES}
)
