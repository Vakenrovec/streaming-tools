#include "catch.hpp"
#include "audio/RecordAudioProcessor.h"
#include "audio/PlaybackAudioProcessor.h"
#include "MediaPacket.h"
#include <Logger.h>
#include <memory>
#include <string>
#include <SDL2/SDL.h>

TEST_CASE("audio", "[record][playback][audio]") {
    REQUIRE_FALSE(SDL_Init(SDL_INIT_AUDIO));
    
    SECTION("Record-and-playback-audio")
    {
        auto recorder = std::make_shared<RecordAudioProcessor>();
        auto playback = std::make_shared<PlaybackAudioProcessor>();
        recorder->SetNextProcessor(playback);
        recorder->Init();

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
