set (PROJECT_NAME core-lib)

set(HEADERS 
    Image.h
    MediaPacket.h
    codecs/vp8.h
    devices/WebCamera.h
    rtp/RtpDefs.h
    rtp/RTPHelper.h
    rtp/RTPFragmenter.h
    # rtp/RTPVp8DepayDataProcessor.h
    rtp/FrameUtils.h
)

set(SOURCES
    codecs/vp8.cpp
    devices/WebCamera.cpp
    rtp/RTPHelper.cpp
    rtp/RTPFragmenter.cpp
    # rtp/RTPVp8DepayDataProcessor.cpp
    rtp/FrameUtils.cpp
)

source_group("Source Files" FILES ${SOURCES})
source_group("Header Files" FILES ${HEADERS})

set(PROJECT_CONTENT ${SOURCES} ${HEADERS})

add_library(${PROJECT_NAME} STATIC ${PROJECT_CONTENT})

target_include_directories(${PROJECT_NAME}
    PRIVATE
        .
        ../utils-lib
        ../network-lib
        ${FFMPEG_INCLUDE_DIRS}
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        utils-lib
        network-lib
        # ${FFMPEG_LIBRARIES}
)
