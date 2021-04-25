#include "catch.hpp"
#include "audio/RecordAudioProcessor.h"
#include "audio/PlaybackAudioProcessor.h"
#include "MediaPacket.h"
#include <Logger.h>
#include <memory>
#include <string>
#include <boost/circular_buffer.hpp>

TEST_CASE("Record-and-playback-audio", "[record][playback][audio]") {
    if (SDL_Init(SDL_INIT_AUDIO) < 0)
    {
        LOG_EX_WARN("SDL could not initialize! SDL Error:" + std::string(SDL_GetError()));
        return;
    }
    
    auto circularBuffer = std::make_shared<boost::circular_buffer<media_packet_ptr>>(10);

    auto recorder = std::make_shared<RecordAudioProcessor>(circularBuffer);
    auto playback = std::make_shared<PlaybackAudioProcessor>(circularBuffer);
    recorder->SetNextProcessor(playback);
    recorder->Init();
    recorder->Play();

    bool quit = false;
    SDL_Event e;
    while(!quit)
    {
        while( SDL_WaitEvent(&e) != 0 )
        // while( SDL_PollEvent(&e) != 0 )
        {
            if( e.type == SDL_QUIT )
            {
                quit = true;
            }

            break;
        }
    }

    recorder->Destroy();
    SDL_Quit();
}
