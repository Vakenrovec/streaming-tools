set (PROJECT_NAME core-lib)

set(HEADERS 
    MediaPacket.h
    DataProcessor.h
    PlayableDataProcessor.h
    utility/QueueDataProcessor.h
    utility/ForkDataProcessor.h
    utility/AudioVideoForkDataProcessor.h
    utility/DelayDataProcessor.h
    codecs/VP8EncoderProcessor.h
    codecs/VP8DecoderProcessor.h
    codecs/OPUSEncoderProcessor.h
    codecs/OPUSDecoderProcessor.h
    rtp/RtpDefs.h
    rtp/RTPHelper.h
    rtp/RTPFragmenterProcessor.h
    rtp/RTPDefragmenterProcessor.h
    rtp/RTPVp8DepayProcessor.h
    rtp/RTPOpusDepayProcessor.h
    rtp/FrameUtils.h
    video/WebCameraProcessor.h
    video/VideoDisplayProcessor.h
    audio/RecordAudioProcessor.h
    audio/PlaybackAudioProcessor.h
    file/FileSaveProcessor.h
    file/FileReadProcessor.h
    file/FileReadRawStreamProcessor.h
    file/FileSaveRawStreamProcessor.h
    image/JPEG2YV12Processor.h
    pipeline/PipelineHelper.h
)

set(SOURCES
    DataProcessor.cpp
    PlayableDataProcessor.cpp
    codecs/VP8EncoderProcessor.cpp
    codecs/VP8DecoderProcessor.cpp
    codecs/OPUSEncoderProcessor.cpp
    codecs/OPUSDecoderProcessor.cpp
    rtp/RTPHelper.cpp
    rtp/RTPFragmenterProcessor.cpp
    rtp/RTPDefragmenterProcessor.cpp
    rtp/RTPVp8DepayProcessor.cpp
    rtp/RTPOpusDepayProcessor.cpp
    rtp/FrameUtils.cpp
    video/WebCameraProcessor.cpp
    video/VideoDisplayProcessor.cpp
    audio/RecordAudioProcessor.cpp
    audio/PlaybackAudioProcessor.cpp
    file/FileSaveProcessor.cpp
    file/FileReadProcessor.cpp
    image/JPEG2YV12Processor.cpp
    pipeline/PipelineHelper.cpp
)

source_group("Source Files" FILES ${SOURCES})
source_group("Header Files" FILES ${HEADERS})

set(PROJECT_CONTENT ${SOURCES} ${HEADERS})

add_library(${PROJECT_NAME} STATIC ${PROJECT_CONTENT})

target_include_directories(${PROJECT_NAME}
    PRIVATE
        .
        ../credentials-lib
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
