#include "receiver.h"
#include "ReceiverSessionProcessor.h"
#include "rtp/RTPVp8DepayProcessor.h"
#include "rtp/RTPDefragmenterProcessor.h"
#include "codecs/VP8DecoderProcessor.h"
#include "video/VideoDisplayProcessor.h"
#include "Logger.h"
#include <SDL2/SDL.h>
#include <chrono>

Receiver::Receiver()
{
    m_ioVideoContext = std::make_shared<boost::asio::io_context>();
    // m_videoWork = std::make_shared<boost::asio::io_context::work>(*m_ioVideoContext);

    m_ioAudioContext = std::make_shared<boost::asio::io_context>();
    m_audioWork = std::make_shared<boost::asio::io_context::work>(*m_ioAudioContext);
}

void Receiver::Start()
{
    int ret = SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER);
    if (ret != 0)
    {
        LOG_EX_WARN("Could not initialize SDL - %s", SDL_GetError());
        return;
    }

    auto receiverSession = std::make_shared<ReceiverSessionProcessor>(*m_ioVideoContext, m_streamId);
    receiverSession->SetServerTcpEndpoint(m_serverTcpIp, m_serverTcpPort);
    receiverSession->SetServerUdpEndpoint(m_serverUdpIp, m_serverUdpPort);
    receiverSession->SetLocalUdpEndpoint(m_localUdpIp, m_localUdpPort);
    auto depay = std::make_shared<RTPVp8DepayProcessor>();
    auto defragmenter = std::make_shared<RTPDefragmenterProcessor>();
    auto decoder = std::make_shared<VP8DecoderProcessor>();
    auto display = std::make_shared<VideoDisplayProcessor>(m_width, m_height);

    receiverSession->SetNextProcessor(depay);
    depay->SetNextProcessor(defragmenter);
    defragmenter->SetNextProcessor(decoder);
    decoder->SetNextProcessor(display);

    m_firstProcessor = receiverSession;
    m_lastProcessor = display;

    receiverSession->Init();   
    m_ioVideoContext->run();
    // m_videoThread = std::make_shared<std::thread>([this, that = shared_from_this()](){
    //     this->m_ioVideoContext->run();
    // });

    while (receiverSession->GetSessionState() != ReceiverSessionProcessor::ReceiverSessionState::CONNECTED)
    {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    
    receiverSession->Play();
    m_ioVideoContext->restart();
    m_ioVideoContext->run();
}

void Receiver::HandleEvents()
{

}

void Receiver::Destroy()
{
    m_videoWork.reset();
    m_ioVideoContext->stop();
    m_ioVideoContext.reset();    
    m_videoThread->join();
    m_firstProcessor->Destroy();
    SDL_Quit();
}
