#include "streamer.h"
#include "video/WebCameraProcessor.h"
#include "image/JPEG2YV12Processor.h"
#include "codecs/VP8EncoderProcessor.h"
#include "rtp/RTPFragmenterProcessor.h"
#include "StreamerSessionProcessor.h"
#include "Logger.h"

Streamer::Streamer()
{
    m_ioVideoContext = std::make_shared<boost::asio::io_context>();
    m_videoWork = std::make_shared<boost::asio::io_context::work>(*m_ioVideoContext);

    m_ioAudioContext = std::make_shared<boost::asio::io_context>();
    m_audioWork = std::make_shared<boost::asio::io_context::work>(*m_ioAudioContext);
}

void Streamer::Start()
{
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

    webcam->Init();

    m_videoThread = std::make_shared<std::thread>([this, that = shared_from_this()](){
        this->m_ioVideoContext->run();
    });

    webcam->Play();
}

void Streamer::HandleEvents()
{

}

void Streamer::Destroy()
{
    m_videoWork.reset();
    m_ioVideoContext->stop();
    m_ioVideoContext.reset();    
    m_videoThread->join();
    m_firstProcessor->Destroy();
}
