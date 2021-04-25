set (PROJECT_NAME core-lib)

set(HEADERS 
    Image.h
    MediaPacket.h
    DataProcessor.h
    codecs/VP8EncoderProcessor.h
    codecs/VP8DecoderProcessor.h
    devices/WebCamera.h
    rtp/RtpDefs.h
    rtp/RTPHelper.h
    rtp/RTPFragmenterProcessor.h
    rtp/RTPDefragmenterProcessor.h
    rtp/RTPVp8Depay.h
    rtp/FrameUtils.h
    video/VideoDisplay.h
    audio/RecordAudioProcessor.h
    audio/PlaybackAudioProcessor.h
    file/FileSaveProcessor.h
    file/FileReadProcessor.h
)

set(SOURCES
    DataProcessor.cpp
    codecs/VP8EncoderProcessor.cpp
    codecs/VP8DecoderProcessor.cpp
    devices/WebCamera.cpp
    rtp/RTPHelper.cpp
    rtp/RTPFragmenterProcessor.cpp
    rtp/RTPDefragmenterProcessor.cpp
    rtp/RTPVp8Depay.cpp
    rtp/FrameUtils.cpp
    video/VideoDisplay.cpp
    audio/RecordAudioProcessor.cpp
    audio/PlaybackAudioProcessor.cpp
    file/FileSaveProcessor.cpp
    file/FileReadProcessor.cpp
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
        ${SDL2_INCLUDE_DIRS}
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        utils-lib
        #network-lib
        # ${FFMPEG_LIBRARIES}
        ${SDL2_LINK_LIBRARIES}
)
