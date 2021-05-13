#include "program.h"
#include "rtp/RTPVp8DepayProcessor.h"
#include "rtp/RTPDefragmenterProcessor.h"
#include "codecs/VP8DecoderProcessor.h"
#include "video/VideoDisplayProcessor.h"
#include "utility/QueueDataProcessor.h"
#include "utility/AudioVideoForkDataProcessor.h"
#include "utility/DelayDataProcessor.h"
#include "rtp/RTPOpusDepayProcessor.h"
#include "codecs/OPUSDecoderProcessor.h"
#include "audio/PlaybackAudioProcessor.h"
#include "file/FileReadRawStreamProcessor.h"
#include "Logger.h"
#include <thread>
#include <iostream>
#include <SDL2/SDL.h>
#include <boost/program_options.hpp>

using namespace boost::program_options;

int main(int argc, char* argv[]) {
    int width = 1280, height = 720, framesDelay = 0;

    std::string rawStreamDir = "/tmp/streams";
    std::string rawStreamFilename = "stream.raw";

    options_description desc("Allowed options");
    desc.add_options()
        ("help,h", "Prints this help")
        ("raw-stream-dir", value<std::string>()->default_value(rawStreamDir), "Raw stream direction")
        ("raw-stream-filename", value<std::string>()->default_value(rawStreamFilename), "Raw stream filename")
    ;

    try {
        variables_map vm;
        store(parse_command_line(argc, argv, desc), vm);
        notify(vm);

        if (vm.count("help")) {  
            std::cout << desc << "\n";
            return 0;
        }

        if (vm.count("raw-stream-dir")) {
            rawStreamDir = vm["raw-stream-dir"].as<std::string>();
        }
        if (vm.count("raw-stream-filename")) {
            rawStreamFilename = vm["raw-stream-filename"].as<std::string>();
        }
        
        SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_EVENTS);

        auto reader = std::make_shared<FileReadRawStreamProcessor<udp_packet_ptr>>(rawStreamDir, rawStreamFilename);
        auto fork = std::make_shared<AudioVideoForkDataProcessor<2>>();

        auto videoQueue = std::make_shared<QueueDataProcessor<udp_packet_ptr>>(0, false);
        auto videoDepay = std::make_shared<RTPVp8DepayProcessor>();
        auto videoDefragmenter = std::make_shared<RTPDefragmenterProcessor>(media_packet_type_t::VP8);
        auto videoDecoder = std::make_shared<VP8DecoderProcessor>();
        // auto videoDelayQueue = std::make_shared<QueueDataProcessor<media_packet_ptr>>(framesDelay, false);
        auto videoDelay = std::make_shared<DelayDataProcessor>();
        auto display = std::make_shared<VideoDisplayProcessor>(width, height);

        auto audioQueue = std::make_shared<QueueDataProcessor<udp_packet_ptr>>(0, false);
        auto audioDepay = std::make_shared<RTPOpusDepayProcessor>();
        auto audioDefragmenter = std::make_shared<RTPDefragmenterProcessor>(media_packet_type_t::OPUS);
        auto audioDecoder = std::make_shared<OPUSDecoderProcessor>();
        auto audioDelay = std::make_shared<DelayDataProcessor>();
        auto playback = std::make_shared<PlaybackAudioProcessor>();

        reader->SetNextProcessor(fork);
        fork->SetNextProcessors({ { audioQueue, videoQueue } });

        audioQueue->SetNextProcessor(audioDepay);
        audioDepay->SetNextProcessor(audioDefragmenter);
        audioDefragmenter->SetNextProcessor(audioDecoder);
        audioDecoder->SetNextProcessor(audioDelay);
        audioDelay->SetNextProcessor(playback);
        
        videoQueue->SetNextProcessor(videoDepay);
        videoDepay->SetNextProcessor(videoDefragmenter);
        videoDefragmenter->SetNextProcessor(videoDecoder);
        // videoDelayQueue->SetNextProcessor(display);
        videoDecoder->SetNextProcessor(videoDelay);
        videoDelay->SetNextProcessor(display);

        reader->Init();
        std::thread mediaplayerThread([&reader, &width, &height](){
            reader->Play();
        });    
        SDL_Event e;
        while (SDL_WaitEvent(&e) != 0)
        {
            if (e.type == SDL_QUIT)
            {
                reader->Stop();
                LOG_EX_INFO("Mediaplayer stopped");
                break;
            }
        }
        mediaplayerThread.join();
        reader->Destroy();

        SDL_Quit();
        LOG_EX_INFO_WITH_CONTEXT("Mediaplayer destroyed");
    } catch (const std::exception& e) {
        std::cerr << e.what() << "\n";
        return 1;
    } catch (...) {
        std::cerr << "Unknown exception\n";
        return 1;
    }

    return 0;
}
