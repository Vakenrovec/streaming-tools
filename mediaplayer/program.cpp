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
#include <SDL2/SDL.h>

int main(int argc, char* argv[]) {
    int width = 1280, height = 720, bitrate = 4000000, delay = 0;
    std::string dir = "/tmp/streams", filename = "stream.raw";

    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_EVENTS);

    auto reader = std::make_shared<FileReadRawStreamProcessor<udp_packet_ptr>>(dir, filename);
    auto fork = std::make_shared<AudioVideoForkDataProcessor<2>>();

    auto videoQueue = std::make_shared<QueueDataProcessor<udp_packet_ptr>>();
    auto videoDepay = std::make_shared<RTPVp8DepayProcessor>();
    auto videoDefragmenter = std::make_shared<RTPDefragmenterProcessor>(media_packet_type_t::VP8);
    auto videoDecoder = std::make_shared<VP8DecoderProcessor>();
    auto videoDelayQueue = std::make_shared<QueueDataProcessor<media_packet_ptr>>(delay);
    auto videoDelay = std::make_shared<DelayDataProcessor>();
    auto display = std::make_shared<VideoDisplayProcessor>(width, height);

    auto audioQueue = std::make_shared<QueueDataProcessor<udp_packet_ptr>>();
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
    videoDelayQueue->SetNextProcessor(display);
    videoDecoder->SetNextProcessor(videoDelay);
    videoDelay->SetNextProcessor(display);

    reader->Init();
    reader->Play();
    reader->Destroy();

    SDL_Quit();

    return 0;
}
