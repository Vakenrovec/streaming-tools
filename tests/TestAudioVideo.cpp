#include "catch.hpp"
#include "codecs/VP8EncoderProcessor.h"
#include "codecs/VP8DecoderProcessor.h"
#include "codecs/OPUSEncoderProcessor.h"
#include "codecs/OPUSDecoderProcessor.h"
#include "file/FileSaveProcessor.h"
#include "file/FileReadProcessor.h"
#include "rtp/RTPFragmenterProcessor.h"
#include "rtp/RTPDefragmenterProcessor.h"
#include "rtp/RTPVp8DepayProcessor.h"
#include "rtp/RTPOpusDepayProcessor.h"
#include "video/WebCameraProcessor.h"
#include "video/VideoDisplayProcessor.h"
#include "image/JPEG2YV12Processor.h"
#include "StreamerSessionProcessor.h"
#include "ReceiverSessionProcessor.h"
#include "utility/QueueDataProcessor.h"
#include "utility/AudioVideoForkDataProcessor.h"
#include "audio/RecordAudioProcessor.h"
#include "audio/PlaybackAudioProcessor.h"
#include "file/FileSaveRawStreamProcessor.h"
#include "file/FileReadRawStreamProcessor.h"
#include "UDPPacket.h"
#include "Logger.h"
#include <memory>
#include <string>
#include <cstdint>
#include <thread>
#include <boost/asio.hpp>
#include <SDL2/SDL.h>

TEST_CASE("audiovideo", "[audio][video][save]")
{
    std::uint32_t streamId = 777;
    int width = 1280, height = 720, gopSize = 10, bitrate = 4000000, want = 30;
    std::string filename = "/tmp/1.jt";
    REQUIRE_FALSE(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_EVENTS));

    SECTION("Audio-video") 
    {
        auto recorder = std::make_shared<RecordAudioProcessor>();
        auto recorderAudioQueue = std::make_shared<QueueDataProcessor<media_packet_ptr>>();
        auto audioEncoder = std::make_shared<OPUSEncoderProcessor>();
        auto audioFragmenter = std::make_shared<RTPFragmenterProcessor>(udp_packet_type_t::RTP_AUDIO);

        auto webcam = std::make_shared<WebCameraProcessor>(width, height);
        auto jpeg2yv12 = std::make_shared<JPEG2YV12Processor>(width, height);
        auto videoEncoder = std::make_shared<VP8EncoderProcessor>(width, height, gopSize, bitrate);
        auto videoFragmenter = std::make_shared<RTPFragmenterProcessor>(udp_packet_type_t::RTP_VIDEO);

        auto fork = std::make_shared<AudioVideoForkDataProcessor<2>>();

        auto audioQueue = std::make_shared<QueueDataProcessor<udp_packet_ptr>>();
        auto audioDepay = std::make_shared<RTPOpusDepayProcessor>();
        auto audioDefragmenter = std::make_shared<RTPDefragmenterProcessor>(media_packet_type_t::OPUS);
        auto audioDecoder = std::make_shared<OPUSDecoderProcessor>();
        auto playback = std::make_shared<PlaybackAudioProcessor>();

        auto videoQueue = std::make_shared<QueueDataProcessor<udp_packet_ptr>>();
        auto videoDepay = std::make_shared<RTPVp8DepayProcessor>();
        auto videoDefragmenter = std::make_shared<RTPDefragmenterProcessor>(media_packet_type_t::VP8);
        auto videoDecoder = std::make_shared<VP8DecoderProcessor>();
        auto display = std::make_shared<VideoDisplayProcessor>(width, height);

        recorder->SetNextProcessor(recorderAudioQueue);
        recorderAudioQueue->SetNextProcessor(audioEncoder);
        audioEncoder->SetNextProcessor(audioFragmenter);
        audioFragmenter->SetNextProcessor(fork);

        webcam->SetNextProcessor(jpeg2yv12);
        jpeg2yv12->SetNextProcessor(videoEncoder);
        videoEncoder->SetNextProcessor(videoFragmenter);
        videoFragmenter->SetNextProcessor(fork);

        fork->SetNextProcessors({ { audioQueue, videoQueue } });

        audioQueue->SetNextProcessor(audioDepay);
        audioDepay->SetNextProcessor(audioDefragmenter);
        audioDefragmenter->SetNextProcessor(audioDecoder);
        audioDecoder->SetNextProcessor(playback);

        videoQueue->SetNextProcessor(videoDepay);
        videoDepay->SetNextProcessor(videoDefragmenter);
        videoDefragmenter->SetNextProcessor(videoDecoder);
        videoDecoder->SetNextProcessor(display);

        audioFragmenter->SetNextProcessor(nullptr);
        recorder->Init();
        audioFragmenter->SetNextProcessor(fork);
        webcam->Init();

        REQUIRE(webcam->Play(want) == want);

        audioFragmenter->SetNextProcessor(nullptr);
        recorder->Destroy();
        webcam->Destroy();
    }

    SECTION("Video-save-raw-stream") 
    {
        auto webcam = std::make_shared<WebCameraProcessor>(width, height);
        auto jpeg2yv12 = std::make_shared<JPEG2YV12Processor>(width, height);
        auto videoEncoder = std::make_shared<VP8EncoderProcessor>(width, height, gopSize, bitrate);
        auto saver = std::make_shared<FileSaveRawStreamProcessor<media_packet_ptr>>(filename);
        auto reader = std::make_shared<FileReadRawStreamProcessor<media_packet_ptr>>(filename);
        auto videoDecoder = std::make_shared<VP8DecoderProcessor>();
        auto display = std::make_shared<VideoDisplayProcessor>(width, height);

        webcam->SetNextProcessor(jpeg2yv12);
        jpeg2yv12->SetNextProcessor(videoEncoder);
        videoEncoder->SetNextProcessor(saver);
        saver->SetNextProcessor(reader);
        reader->SetNextProcessor(videoDecoder);
        videoDecoder->SetNextProcessor(display);

        webcam->Init();
        REQUIRE(webcam->Play(want) == want);
        webcam->Destroy();
    }

    SECTION("Audio-save-raw-stream") 
    {
        auto recorder = std::make_shared<RecordAudioProcessor>();
        auto audioEncoder = std::make_shared<OPUSEncoderProcessor>();
        auto saver = std::make_shared<FileSaveRawStreamProcessor<media_packet_ptr>>(filename);
        auto reader = std::make_shared<FileReadRawStreamProcessor<media_packet_ptr>>(filename);
        auto audioDecoder = std::make_shared<OPUSDecoderProcessor>();
        auto playback = std::make_shared<PlaybackAudioProcessor>();

        recorder->SetNextProcessor(audioEncoder);
        audioEncoder->SetNextProcessor(saver);
        saver->SetNextProcessor(reader);
        reader->SetNextProcessor(audioDecoder);
        audioDecoder->SetNextProcessor(playback);

        recorder->Init();
        std::this_thread::sleep_for(std::chrono::seconds(5));
        recorder->Destroy();
    }

    SDL_Quit();
}
