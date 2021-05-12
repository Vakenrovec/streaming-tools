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
    int width = 1280, height = 720, gopSize = 10, bitrate = 4000000;
    int want = 50, framesDelay = 10;
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
        
        auto videoQueue = std::make_shared<QueueDataProcessor<udp_packet_ptr>>();
        auto videoDepay = std::make_shared<RTPVp8DepayProcessor>();
        auto videoDefragmenter = std::make_shared<RTPDefragmenterProcessor>(media_packet_type_t::VP8);
        auto videoDecoder = std::make_shared<VP8DecoderProcessor>();
        // auto videoDelayQueue = std::make_shared<QueueDataProcessor<media_packet_ptr>>(delay);
        auto display = std::make_shared<VideoDisplayProcessor>(width, height);
        
        auto audioQueue = std::make_shared<QueueDataProcessor<udp_packet_ptr>>();
        auto audioDepay = std::make_shared<RTPOpusDepayProcessor>();
        auto audioDefragmenter = std::make_shared<RTPDefragmenterProcessor>(media_packet_type_t::OPUS);
        auto audioDecoder = std::make_shared<OPUSDecoderProcessor>();
        auto playback = std::make_shared<PlaybackAudioProcessor>();
        
        receiver->SetNextProcessor(fork);
        fork->SetNextProcessors({ { audioQueue, videoQueue } });

        videoQueue->SetNextProcessor(videoDepay);
        videoDepay->SetNextProcessor(videoDefragmenter);
        videoDefragmenter->SetNextProcessor(videoDecoder);
        videoDecoder->SetNextProcessor(display);
        // videoDelayQueue->SetNextProcessor(display);

        audioQueue->SetNextProcessor(audioDepay);
        audioDepay->SetNextProcessor(audioDefragmenter);
        audioDefragmenter->SetNextProcessor(audioDecoder);
        audioDecoder->SetNextProcessor(playback);

        fork->Init();

        receiver->Play();
        std::thread receiverWorker([&](){
            ioReceiverContext->run();
        });
        
        //////////////////////////////////////////////////////////////////

        auto recorder = std::make_shared<RecordAudioProcessor>();
        auto recorderAudioQueue = std::make_shared<QueueDataProcessor<media_packet_ptr>>();
        auto audioEncoder = std::make_shared<OPUSEncoderProcessor>();
        auto audioFragmenter = std::make_shared<RTPFragmenterProcessor>(udp_packet_type_t::RTP_AUDIO);

        auto webcam = std::make_shared<WebCameraProcessor>(width, height);
        auto jpeg2yv12 = std::make_shared<JPEG2YV12Processor>(width, height);
        auto videoEncoder = std::make_shared<VP8EncoderProcessor>(width, height, gopSize, bitrate);
        auto videoDelayQueue = std::make_shared<QueueDataProcessor<media_packet_ptr>>(framesDelay);
        auto videoFragmenter = std::make_shared<RTPFragmenterProcessor>(udp_packet_type_t::RTP_VIDEO);

        auto streamer = std::make_shared<StreamerSessionProcessor>(*ioStreamerContext, streamId);
        streamer->SetLocalUdpEndpoint("192.11.0.3", 35006);
        streamer->SetServerUdpEndpoint("192.11.0.3", 35007);
        streamer->GetUdpSocket()->open(streamer->GetLocalUdpEndpoint().protocol());
        streamer->GetUdpSocket()->bind(streamer->GetLocalUdpEndpoint());
        streamer->SetSessionState(StreamerSessionProcessor::StreamerSessionState::SESSION_CREATED);
        
        recorder->SetNextProcessor(recorderAudioQueue);
        recorderAudioQueue->SetNextProcessor(audioEncoder);
        audioEncoder->SetNextProcessor(audioFragmenter);
        recorder->Init();
        audioFragmenter->SetNextProcessor(streamer);

        webcam->SetNextProcessor(jpeg2yv12);
        jpeg2yv12->SetNextProcessor(videoEncoder);
        videoEncoder->SetNextProcessor(videoDelayQueue);
        videoDelayQueue->SetNextProcessor(videoDefragmenter);
        webcam->Init();
        videoFragmenter->SetNextProcessor(streamer);

        auto ioStreamerServiceWork = std::make_shared<boost::asio::io_context::work>(*ioStreamerContext);
        std::thread streamerWorker([&](){
            ioStreamerContext->run();
        });
        REQUIRE(webcam->Play(want) == want);
        ioStreamerServiceWork.reset();
        streamerWorker.join();
        videoFragmenter->SetNextProcessor(nullptr);
        webcam->Destroy();
        audioFragmenter->SetNextProcessor(nullptr);
        recorder->Destroy();

        //////////////////////////////////////////////////////////////////

        ioReceiverContext->stop();
        receiverWorker.join();
        fork->Destroy();
    }

    SDL_Quit();
}
