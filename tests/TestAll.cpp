#include "catch.hpp"
#include "codecs/VP8EncoderProcessor.h"
#include "codecs/VP8DecoderProcessor.h"
#include "file/FileSaveProcessor.h"
#include "file/FileReadProcessor.h"
#include "rtp/RTPFragmenterProcessor.h"
#include "rtp/RTPDefragmenterProcessor.h"
#include "rtp/RTPVp8DepayProcessor.h"
#include "video/WebCameraProcessor.h"
#include "video/VideoDisplayProcessor.h"
#include "image/JPEG2YV12Processor.h"
#include "StreamerSessionProcessor.h"
#include "ReceiverSessionProcessor.h"
#include "utility/QueueDataProcessor.h"
#include "utility/AudioVideoForkDataProcessor.h"
#include "audio/RecordAudioProcessor.h"
#include "audio/PlaybackAudioProcessor.h"
#include "UDPPacket.h"
#include "Logger.h"
#include <memory>
#include <cstdint>
#include <thread>
#include <boost/asio.hpp>
#include <SDL2/SDL.h>

TEST_CASE("all", "[network][audio][video][all]")
{
    std::uint32_t streamId = 777;
    int width = 1280, height = 720, gopSize = 10, bitrate = 4000000, want = 50;
    REQUIRE_FALSE(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_EVENTS));

    auto ioStreamerContext = std::make_shared<boost::asio::io_context>();
    auto ioReceiverContext = std::make_shared<boost::asio::io_context>();

    SECTION("Audio-video-communication") 
    {
        auto receiver = std::make_shared<ReceiverSessionProcessor>(*ioReceiverContext, streamId);
        receiver->SetLocalUdpEndpoint("192.11.0.3", 35007);
        receiver->SetServerUdpEndpoint("192.11.0.3", 35006);
        receiver->GetUdpSocket()->open(receiver->GetLocalUdpEndpoint().protocol());
        receiver->GetUdpSocket()->bind(receiver->GetLocalUdpEndpoint());
        receiver->SetSessionState(ReceiverSessionProcessor::ReceiverSessionState::CONNECTED);
        auto fork = std::make_shared<AudioVideoForkDataProcessor<2>>();
        auto queue = std::make_shared<QueueDataProcessor<udp_packet_ptr>>();
        auto depay = std::make_shared<RTPVp8DepayProcessor>();
        auto defragmenter = std::make_shared<RTPDefragmenterProcessor>();
        auto decoder = std::make_shared<VP8DecoderProcessor>();
        auto display = std::make_shared<VideoDisplayProcessor>(width, height);
        // auto playback = std::make_shared<PlaybackAudioProcessor>();
        receiver->SetNextProcessor(fork);
        fork->SetNextProcessors({ { nullptr, queue } });
        queue->SetNextProcessor(depay);
        depay->SetNextProcessor(defragmenter);
        defragmenter->SetNextProcessor(decoder);
        decoder->SetNextProcessor(display);
        queue->Init();   

        receiver->Play();
        std::thread receiverWorker([&](){
            ioReceiverContext->run();
        });
        
        

        auto recorder = std::make_shared<RecordAudioProcessor>();
        auto webcam = std::make_shared<WebCameraProcessor>(width, height);
        auto jpeg2yv12 = std::make_shared<JPEG2YV12Processor>(width, height);
        auto encoder = std::make_shared<VP8EncoderProcessor>(width, height, gopSize, bitrate);
        auto fragmenter = std::make_shared<RTPFragmenterProcessor>();
        auto streamer = std::make_shared<StreamerSessionProcessor>(*ioStreamerContext, streamId);
        streamer->SetLocalUdpEndpoint("192.11.0.3", 35006);
        streamer->SetServerUdpEndpoint("192.11.0.3", 35007);
        streamer->GetUdpSocket()->open(streamer->GetLocalUdpEndpoint().protocol());
        streamer->GetUdpSocket()->bind(streamer->GetLocalUdpEndpoint());
        streamer->SetSessionState(StreamerSessionProcessor::StreamerSessionState::SESSION_CREATED);
        
        webcam->SetNextProcessor(jpeg2yv12);
        jpeg2yv12->SetNextProcessor(encoder);
        encoder->SetNextProcessor(fragmenter);
        webcam->Init();
        fragmenter->SetNextProcessor(streamer);

        auto ioStreamerServiceWork = std::make_shared<boost::asio::io_context::work>(*ioStreamerContext);
        std::thread streamerWorker([&](){
            ioStreamerContext->run();
        });
        REQUIRE(webcam->Play(want) == want);
        ioStreamerServiceWork.reset();
        streamerWorker.join();
        fragmenter->SetNextProcessor(nullptr);
        webcam->Destroy();

        

        ioReceiverContext->stop();
        receiverWorker.join();
        queue->Destroy();
    }

    SDL_Quit();
}
