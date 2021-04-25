#include "catch.hpp"
#include "codecs/VP8EncoderProcessor.h"
#include "codecs/VP8DecoderProcessor.h"
#include "file/FileSaveProcessor.h"
#include "file/FileReadProcessor.h"
#include "rtp/RTPFragmenterProcessor.h"
#include "rtp/RTPDefragmenterProcessor.h"
#include "FileUtils.h"
#include <string>
#include <cstdint>
#include <memory>

TEST_CASE("Encode-decode-yv12", "[encode][decode]") {
    int width = 1280, height = 720, gopSize = 10, bitrate = 400000, want = 11;
    std::string testFileName = FileUtils::CombinePath(FileUtils::GetCurrentExecutableFolder(), "test-data/webcam_output.yuv420p");
    std::string saveFileName = FileUtils::CombinePath(FileUtils::GetCurrentExecutableFolder(), "test-result/webcam_output.yv12");
    
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

TEST_CASE("Encode-fragment-defragment-decode-yv12", "[encode][fragment][defragment][decode]") {
    int width = 1280, height = 720, gopSize = 10, bitrate = 400000, want = 11;
    std::string testFileName = FileUtils::CombinePath(FileUtils::GetCurrentExecutableFolder(), "test-data/webcam_output.yuv420p");
    std::string saveFileName = FileUtils::CombinePath(FileUtils::GetCurrentExecutableFolder(), "test-result/webcam_output.yv12");
    
    auto reader = std::make_shared<FileReadProcessor>(testFileName);
    auto encoder = std::make_shared<VP8EncoderProcessor>(width, height, gopSize, bitrate);
    auto fragmenter = std::make_shared<RTPFragmenterProcessor>();
    auto defragmenter = std::make_shared<RTPDefragmenterProcessor>();
    auto decoder = std::make_shared<VP8DecoderProcessor>();
    auto saver = std::make_shared<FileSaveProcessor>(saveFileName);

    reader->SetNextProcessor(encoder);
    encoder->SetNextProcessor(fragmenter);
    fragmenter->SetNextProcessor(defragmenter);
    defragmenter->SetNextProcessor(decoder);
    decoder->SetNextProcessor(saver);

    reader->Init();
    REQUIRE(reader->Play(want) == want);
    reader->Destroy();
}
