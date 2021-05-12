#include "catch.hpp"
#include "audio/RecordAudioProcessor.h"
#include "audio/PlaybackAudioProcessor.h"
#include "rtp/RTPFragmenterProcessor.h"
#include "rtp/RTPDefragmenterProcessor.h"
#include "rtp/RTPVp8DepayProcessor.h"
#include "rtp/RTPOpusDepayProcessor.h"
#include "codecs/OPUSEncoderProcessor.h"
#include "codecs/OPUSDecoderProcessor.h"
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

    SECTION("Record-fragment-depay-defragment-playback-audio")
    {
        auto recorder = std::make_shared<RecordAudioProcessor>();
        auto fragmenter = std::make_shared<RTPFragmenterProcessor>(udp_packet_type_t::RTP_AUDIO);
        auto depay = std::make_shared<RTPOpusDepayProcessor>();
        auto defragmenter = std::make_shared<RTPDefragmenterProcessor>(media_packet_type_t::OPUS);
        auto playback = std::make_shared<PlaybackAudioProcessor>();
        recorder->SetNextProcessor(fragmenter);
        fragmenter->SetNextProcessor(depay);
        depay->SetNextProcessor(defragmenter);
        defragmenter->SetNextProcessor(playback);
        recorder->Init();

        bool quit = false;
        SDL_Event e;
        while (!quit)
        {
            while (SDL_WaitEvent(&e) != 0)
            {
                if (e.type == SDL_QUIT)
                {
                    quit = true;
                }

                break;
            }
        }
        recorder->Destroy();
    }

    SECTION("Record-encode-fragment-depay-defragment-decode-playback-audio")
    {
        auto recorder = std::make_shared<RecordAudioProcessor>();
        auto encoder = std::make_shared<OPUSEncoderProcessor>();
        auto fragmenter = std::make_shared<RTPFragmenterProcessor>(udp_packet_type_t::RTP_AUDIO);
        auto depay = std::make_shared<RTPOpusDepayProcessor>();
        auto defragmenter = std::make_shared<RTPDefragmenterProcessor>(media_packet_type_t::OPUS);
        auto decoder = std::make_shared<OPUSDecoderProcessor>();
        auto playback = std::make_shared<PlaybackAudioProcessor>();

        recorder->SetNextProcessor(encoder);
        encoder->SetNextProcessor(fragmenter);
        fragmenter->SetNextProcessor(depay);
        depay->SetNextProcessor(defragmenter);
        defragmenter->SetNextProcessor(decoder);
        decoder->SetNextProcessor(playback);
        recorder->Init();

        bool quit = false;
        SDL_Event e;
        while (!quit)
        {
            while (SDL_WaitEvent(&e) != 0)
            {
                if (e.type == SDL_QUIT)
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
