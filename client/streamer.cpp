#include "streamer.h"
#include "PlayableDataProcessor.h"
#include "video/WebCameraProcessor.h"
#include "image/JPEG2YV12Processor.h"
#include "codecs/VP8EncoderProcessor.h"
#include "rtp/RTPFragmenterProcessor.h"
#include "StreamerSessionProcessor.h"
#include "Logger.h"
#include <SDL2/SDL.h>

Streamer::Streamer()
{
    m_ioVideoContext = std::make_shared<boost::asio::io_context>();
    m_videoWork = std::make_shared<boost::asio::io_context::work>(*m_ioVideoContext);

    // m_ioAudioContext = std::make_shared<boost::asio::io_context>();
    // m_audioWork = std::make_shared<boost::asio::io_context::work>(*m_ioAudioContext);
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
    auto encoder = std::make_shared<VP8EncoderProcessor>(m_width, m_height, m_gopSize, m_bitrate);
    auto fragmenter = std::make_shared<RTPFragmenterProcessor>();
    auto streamerSession = std::make_shared<StreamerSessionProcessor>(*m_ioVideoContext, m_streamId);
    streamerSession->SetServerTcpEndpoint(m_serverTcpIp, m_serverTcpPort);
    streamerSession->SetServerUdpEndpoint(m_serverUdpIp, m_serverUdpPort);
    streamerSession->SetLocalUdpEndpoint(m_localUdpIp, m_localUdpPort);

    webcam->SetNextProcessor(jpeg2yv12);
    jpeg2yv12->SetNextProcessor(encoder);
    encoder->SetNextProcessor(fragmenter);
    fragmenter->SetNextProcessor(streamerSession);

    m_firstProcessor = webcam;
    m_lastProcessor = streamerSession;

    m_videoSenderThread = std::make_shared<std::thread>([this, that = shared_from_this()](){
        this->m_ioVideoContext->run();
    });

    m_videoPlayThread = std::make_shared<std::thread>([this, that = shared_from_this()](){
        auto processor = std::dynamic_pointer_cast<PlayableDataProcessor>(this->m_firstProcessor);
        processor->Init();
        processor->Play();
    });

    LOG_EX_INFO("Streamer started");
}

void Streamer::HandleEvents()
{
    SDL_Event e;
    while(SDL_WaitEvent(&e) != 0)
    {
        if (e.type == SDL_QUIT)
        {
            auto processor = std::dynamic_pointer_cast<PlayableDataProcessor>(this->m_firstProcessor);
            processor->Stop();
            LOG_EX_INFO("Exit from sdl");
            break;
        }
    }
}

void Streamer::Destroy()
{
    m_videoPlayThread->join();
    m_firstProcessor->Destroy();
    m_videoWork.reset();
    m_videoSenderThread->join();
    SDL_Quit();
    LOG_EX_INFO("Streamer destroyed");
}
