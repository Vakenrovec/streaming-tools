#include "catch.hpp"
#include "audio/RecordAudioProcessor.h"
#include "audio/PlaybackAudioProcessor.h"
#include "MediaPacket.h"
#include <Logger.h>
#include <memory>
#include <string>
#include <boost/circular_buffer.hpp>
#include <SDL2/SDL.h>

TEST_CASE("audio", "[record][playback][audio]") {
    REQUIRE_FALSE(SDL_Init(SDL_INIT_AUDIO));
    
    SECTION("Record-and-playback-audio")
    {
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
    }

    SDL_Quit();
}
