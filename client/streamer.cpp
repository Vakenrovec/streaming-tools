#include "streamer.h"
#include "PlayableDataProcessor.h"
#include "video/WebCameraProcessor.h"
#include "image/JPEG2YV12Processor.h"
#include "codecs/VP8EncoderProcessor.h"
#include "rtp/RTPFragmenterProcessor.h"
#include "audio/RecordAudioProcessor.h"
#include "utility/QueueDataProcessor.h"
#include "rtp/RTPOpusDepayProcessor.h"
#include "codecs/OPUSEncoderProcessor.h"
#include "rtp/RTPFragmenterProcessor.h"
#include "StreamerSessionProcessor.h"
#include "Logger.h"
#include <SDL2/SDL.h>

Streamer::Streamer()
: m_framesDelay(10)
{
    m_ioContext = std::make_shared<boost::asio::io_context>();
    m_work = std::make_shared<boost::asio::io_context::work>(*m_ioContext);
}

void Streamer::StartAsync()
{
    if (SDL_Init(SDL_INIT_EVENTS | SDL_INIT_AUDIO))
    {
        LOG_EX_WARN("Could not initialize SDL - %s", SDL_GetError());
        return;
    }

    auto webcam = std::make_shared<WebCameraProcessor>(m_width, m_height);
    auto jpeg2yv12 = std::make_shared<JPEG2YV12Processor>(m_width, m_height);
    auto videoEncoder = std::make_shared<VP8EncoderProcessor>(m_width, m_height, m_gopSize, m_bitrate);
    auto videoDelayQueue = std::make_shared<QueueDataProcessor<media_packet_ptr>>(m_framesDelay);
    auto videoFragmenter = std::make_shared<RTPFragmenterProcessor>(udp_packet_type_t::RTP_VIDEO);
    
    auto recorder = std::make_shared<RecordAudioProcessor>();
    auto audioQueue = std::make_shared<QueueDataProcessor<media_packet_ptr>>();
    auto audioEncoder = std::make_shared<OPUSEncoderProcessor>();
    auto audioFragmenter = std::make_shared<RTPFragmenterProcessor>(udp_packet_type_t::RTP_AUDIO);

    auto streamerSession = std::make_shared<StreamerSessionProcessor>(*m_ioContext, m_streamId);
    streamerSession->SetServerTcpEndpoint(m_serverTcpIp, m_serverTcpPort);
    streamerSession->SetServerUdpEndpoint(m_serverUdpIp, m_serverUdpPort);
    streamerSession->SetLocalUdpEndpoint(m_localUdpIp, m_localUdpPort);

    webcam->SetNextProcessor(jpeg2yv12);
    jpeg2yv12->SetNextProcessor(videoEncoder);
    videoEncoder->SetNextProcessor(videoDelayQueue);
    videoDelayQueue->SetNextProcessor(videoFragmenter);
    videoFragmenter->SetNextProcessor(streamerSession);
    
    recorder->SetNextProcessor(audioQueue);
    audioQueue->SetNextProcessor(audioEncoder);
    audioEncoder->SetNextProcessor(audioFragmenter);
    audioFragmenter->SetNextProcessor(streamerSession);

    m_firstVideoProcessor = webcam;
    m_firstAudioProcessor = recorder;

    m_pipelineSenderThread = std::make_shared<std::thread>([this, that = shared_from_this()](){
        this->m_ioContext->run();
    });

    m_pipelinePlayThread = std::make_shared<std::thread>([this, that = shared_from_this()](){
        if (!this->m_disableAudio) 
        {
            auto audioProcessor = std::dynamic_pointer_cast<DataProcessor>(this->m_firstAudioProcessor);
            audioProcessor->Init();
        }

        if (!m_disableVideo) 
        {
            auto videoProcessor = std::dynamic_pointer_cast<PlayableDataProcessor>(this->m_firstVideoProcessor);
            videoProcessor->Init();
            videoProcessor->Play();
        }
    });

    LOG_EX_INFO("Streamer started");
}

void Streamer::HandleEvents()
{
    SDL_Event e;
    while (SDL_WaitEvent(&e) != 0)
    {
        if (e.type == SDL_QUIT)
        {
            auto processor = std::dynamic_pointer_cast<PlayableDataProcessor>(this->m_firstVideoProcessor);
            processor->Stop();
            LOG_EX_INFO("Exit from sdl");
            break;
        }
    }
}

void Streamer::Destroy()
{
    m_pipelinePlayThread->join();
    m_firstVideoProcessor->Destroy();
    m_firstAudioProcessor->Destroy();
    m_work.reset();
    m_pipelineSenderThread->join();
    SDL_Quit();
    LOG_EX_INFO("Streamer destroyed");
}
