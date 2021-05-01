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
#include "FileUtils.h"
#include "Logger.h"
#include <string>
#include <cstdint>
#include <memory>
#include <SDL2/SDL.h>

TEST_CASE("video", "[video]")
{
    int width = 1280, height = 720, gopSize = 10, bitrate = 4000000, want = 20;
    std::string testFileName = FileUtils::CombinePath(FileUtils::GetCurrentExecutableFolder(), "test-data/webcam_output.yv12");
    std::string saveFileName = FileUtils::CombinePath(FileUtils::GetCurrentExecutableFolder(), "test-result/webcam_output_result.yv12");    
    REQUIRE_FALSE(SDL_Init(SDL_INIT_VIDEO));

    SECTION("WebCamera-convert-display", "[webcam][conveert][display]")
    {
        auto webcam = std::make_shared<WebCameraProcessor>(width, height);
        auto jpeg2yv12 = std::make_shared<JPEG2YV12Processor>(width, height);
        auto display = std::make_shared<VideoDisplayProcessor>(width, height);

        webcam->SetNextProcessor(jpeg2yv12);
        jpeg2yv12->SetNextProcessor(display);

        webcam->Init();
        REQUIRE(webcam->Play(want) == want);
        webcam->Destroy();
    }

    SECTION("Encode-decode", "[encode][decode]") 
    {
        auto reader = std::make_shared<FileReadProcessor>(testFileName);
        auto encoder = std::make_shared<VP8EncoderProcessor>(width, height, gopSize, bitrate);
        auto decoder = std::make_shared<VP8DecoderProcessor>();
        auto saver = std::make_shared<FileSaveProcessor>(saveFileName);
        
        reader->SetNextProcessor(encoder);
        encoder->SetNextProcessor(decoder);
        decoder->SetNextProcessor(saver);

        reader->Init();
        REQUIRE(reader->Play(want) == want);
        reader->Destroy();
    }

    SECTION("Encode-fragment-defragment-decode", "[encode][fragment][defragment][decode]") 
    {
        auto reader = std::make_shared<FileReadProcessor>(testFileName);
        auto encoder = std::make_shared<VP8EncoderProcessor>(width, height, gopSize, bitrate);
        auto fragmenter = std::make_shared<RTPFragmenterProcessor>();
        auto defragmenter = std::make_shared<RTPDefragmenterProcessor>();
        auto decoder = std::make_shared<VP8DecoderProcessor>();
        auto saver = std::make_shared<FileSaveProcessor>(saveFileName);
        auto display = std::make_shared<VideoDisplayProcessor>(width, height);

        reader->SetNextProcessor(encoder);
        encoder->SetNextProcessor(fragmenter);
        fragmenter->SetNextProcessor(defragmenter);
        defragmenter->SetNextProcessor(decoder);
        decoder->SetNextProcessor(saver);
        saver->SetNextProcessor(display);

        reader->Init();
        REQUIRE(reader->Play(want) == want);
        reader->Destroy();
    }

    SECTION("WebCamera-convert-encode-fragment-depay-defragment-decode-display", 
        "[webcam][convert][encode][fragment][depay][defragment][decode][display]") 
    {
        auto webcam = std::make_shared<WebCameraProcessor>(width, height);
        auto jpeg2yv12 = std::make_shared<JPEG2YV12Processor>(width, height);
        auto encoder = std::make_shared<VP8EncoderProcessor>(width, height, gopSize, bitrate);
        auto fragmenter = std::make_shared<RTPFragmenterProcessor>();
        auto depay = std::make_shared<RTPVp8DepayProcessor>();
        auto defragmenter = std::make_shared<RTPDefragmenterProcessor>();
        auto decoder = std::make_shared<VP8DecoderProcessor>();
        auto display = std::make_shared<VideoDisplayProcessor>(width, height);

        webcam->SetNextProcessor(jpeg2yv12);
        jpeg2yv12->SetNextProcessor(encoder);
        encoder->SetNextProcessor(fragmenter);
        fragmenter->SetNextProcessor(depay);
        depay->SetNextProcessor(defragmenter);
        defragmenter->SetNextProcessor(decoder);
        decoder->SetNextProcessor(display);

        webcam->Init();
        REQUIRE(webcam->Play(want) == want);
        webcam->Destroy();
    }

    SDL_Quit();
}
