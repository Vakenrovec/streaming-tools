#include "PipelineHelper.h"
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

Pipeline PipelineHelper::CreateMediaplayer(
    const std::string& rawStreamDir, const std::string& rawStreamFilename,
    const int width, const int height
)
{
    auto reader = std::make_shared<FileReadRawStreamProcessor<udp_packet_ptr>>(rawStreamDir, rawStreamFilename);
    auto fork = std::make_shared<AudioVideoForkDataProcessor<2>>();

    auto audioQueue = std::make_shared<QueueDataProcessor<udp_packet_ptr>>(0, true);
    auto audioDepay = std::make_shared<RTPOpusDepayProcessor>();
    auto audioDefragmenter = std::make_shared<RTPDefragmenterProcessor>(media_packet_type_t::OPUS);
    auto audioDecoder = std::make_shared<OPUSDecoderProcessor>();
    auto audioDelay = std::make_shared<DelayDataProcessor>();
    auto playback = std::make_shared<PlaybackAudioProcessor>();

    auto videoQueue = std::make_shared<QueueDataProcessor<udp_packet_ptr>>(0, true);
    auto videoDepay = std::make_shared<RTPVp8DepayProcessor>();
    auto videoDefragmenter = std::make_shared<RTPDefragmenterProcessor>(media_packet_type_t::VP8);
    auto videoDecoder = std::make_shared<VP8DecoderProcessor>();
    // auto videoDelayQueue = std::make_shared<QueueDataProcessor<media_packet_ptr>>(framesDelay, false);
    auto videoDelay = std::make_shared<DelayDataProcessor>();
    auto display = std::make_shared<VideoDisplayProcessor>(width, height);

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

    auto pipeline = std::make_shared<Pipeline::element_type>();
    pipeline->reserve(14);
    pipeline->push_back(reader); // 0
    pipeline->push_back(fork);
    pipeline->push_back(audioQueue); // 2
    pipeline->push_back(audioDepay);
    pipeline->push_back(audioDefragmenter);
    pipeline->push_back(audioDecoder);
    pipeline->push_back(audioDelay);
    pipeline->push_back(playback);
    pipeline->push_back(videoQueue); // 8
    pipeline->push_back(videoDepay);
    pipeline->push_back(videoDefragmenter);
    pipeline->push_back(videoDecoder);
    pipeline->push_back(videoDelay);
    pipeline->push_back(display);

    return pipeline;
}
