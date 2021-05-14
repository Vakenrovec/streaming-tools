#include "receiver.h"
#include "ReceiverSessionProcessor.h"
#include "rtp/RTPVp8DepayProcessor.h"
#include "rtp/RTPDefragmenterProcessor.h"
#include "codecs/VP8DecoderProcessor.h"
#include "video/VideoDisplayProcessor.h"
#include "utility/QueueDataProcessor.h"
#include "utility/AudioVideoForkDataProcessor.h"
#include "rtp/RTPOpusDepayProcessor.h"
#include "codecs/OPUSDecoderProcessor.h"
#include "audio/PlaybackAudioProcessor.h"
#include "file/FileSaveRawStreamProcessor.h"
#include "Logger.h"
#include <SDL2/SDL.h>
#include <chrono>

Receiver::Receiver()
{
    m_ioContext = std::make_shared<boost::asio::io_context>();
}

void Receiver::StartAsync()
{
    int ret = SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_EVENTS);
    if (ret != 0)
    {
        LOG_EX_WARN_WITH_CONTEXT("Could not initialize SDL - %s", SDL_GetError());
        return;
    }

    auto receiverSession = std::make_shared<ReceiverSessionProcessor>(*m_ioContext, m_streamId);
    receiverSession->SetServerTcpEndpoint(m_serverIp, m_serverTcpPort);
    receiverSession->SetServerUdpEndpoint(m_serverIp, m_serverUdpPort);
    receiverSession->SetLocalUdpEndpoint(m_localIp, m_localUdpPort);
    auto saver = std::make_shared<FileSaveRawStreamProcessor<udp_packet_ptr>>(m_rawStreamDir, m_rawStreamFilename);
    auto fork = std::make_shared<AudioVideoForkDataProcessor<2>>();

    auto videoQueue = std::make_shared<QueueDataProcessor<udp_packet_ptr>>();
    auto videoDepay = std::make_shared<RTPVp8DepayProcessor>();
    auto videoDefragmenter = std::make_shared<RTPDefragmenterProcessor>(media_packet_type_t::VP8);
    auto videoDecoder = std::make_shared<VP8DecoderProcessor>();
    auto display = std::make_shared<VideoDisplayProcessor>(m_width, m_height);

    auto audioQueue = std::make_shared<QueueDataProcessor<udp_packet_ptr>>();
    auto audioDepay = std::make_shared<RTPOpusDepayProcessor>();
    auto audioDefragmenter = std::make_shared<RTPDefragmenterProcessor>(media_packet_type_t::OPUS);
    auto audioDecoder = std::make_shared<OPUSDecoderProcessor>();
    auto playback = std::make_shared<PlaybackAudioProcessor>();

    if (m_saveRawStream)
    {
        receiverSession->SetNextProcessor(saver);
        saver->SetNextProcessor(fork);
    }
    else
    {
        receiverSession->SetNextProcessor(fork);
    }    
    fork->SetNextProcessors({ { 
        !this->m_disableAudio ? audioQueue : nullptr, 
        !this->m_disableVideo ? videoQueue : nullptr
    } });

    videoQueue->SetNextProcessor(videoDepay);
    videoDepay->SetNextProcessor(videoDefragmenter);
    videoDefragmenter->SetNextProcessor(videoDecoder);
    videoDecoder->SetNextProcessor(display);

    audioQueue->SetNextProcessor(audioDepay);
    audioDepay->SetNextProcessor(audioDefragmenter);
    audioDefragmenter->SetNextProcessor(audioDecoder);
    audioDecoder->SetNextProcessor(playback);

    m_firstProcessor = receiverSession;

    receiverSession->Init();   
    m_ioContext->run();
    m_ioContext->restart();
    receiverSession->Play();
    
    m_pipelineReceiverThread = std::make_shared<std::thread>([this, that = shared_from_this()](){
        this->m_ioContext->run();
    });

    LOG_EX_INFO_WITH_CONTEXT("Receiver started");
}

void Receiver::HandleEvents()
{
    SDL_Event e;
    while (SDL_WaitEvent(&e) != 0)
    {
        if (e.type == SDL_QUIT)
        {
            LOG_EX_INFO_WITH_CONTEXT("Got stopped sygnal");
            break;
        }
    }
}

void Receiver::Destroy()
{
    m_ioContext->stop();  
    m_pipelineReceiverThread->join();
    m_firstProcessor->Destroy();
    m_ioContext->restart();
    m_ioContext->run();    
    SDL_Quit();
    LOG_EX_INFO_WITH_CONTEXT("Receiver destroyed");
}
