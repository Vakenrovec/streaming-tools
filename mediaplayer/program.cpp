#include "program.h"
#include "pipeline/PipelineHelper.h"
#include "utility/QueueDataProcessor.h"
#include "PlayableDataProcessor.h"
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

        Pipeline pipeline = PipelineHelper::CreateMediaplayer(rawStreamDir, rawStreamFilename, width, height);
        auto reader = std::dynamic_pointer_cast<PlayableDataProcessor>((*pipeline)[0]);
        auto audioQueue = std::dynamic_pointer_cast<QueueDataProcessor<udp_packet_ptr>>((*pipeline)[2]);
        auto videoQueue = std::dynamic_pointer_cast<QueueDataProcessor<udp_packet_ptr>>((*pipeline)[8]);

        bool end = false;
        reader->Init();
        std::thread mediaplayerThread([reader, audioQueue, videoQueue, &end](){
            reader->Play();
            LOG_EX_INFO("Everything was read");
            audioQueue->Process(nullptr);
            audioQueue->SetState(DataProcessor::State::STOPPING);
            videoQueue->Process(nullptr);
            videoQueue->SetState(DataProcessor::State::STOPPING); 
            reader->Destroy();
            end = true;
        });    
        SDL_Event e;
        std::chrono::milliseconds delay = std::chrono::seconds(1);
        LOG_EX_INFO("%d", delay);
        for (;;)
        {
            int res = SDL_WaitEventTimeout(&e, delay.count());
            if (end)
            {
                break;
            }
            if (e.type == SDL_QUIT)
            {
                reader->Stop();
                videoQueue->Clear();
                audioQueue->Clear();                
                LOG_EX_INFO("Mediaplayer stopped");
                break;
            }            
        }
        mediaplayerThread.join();

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
