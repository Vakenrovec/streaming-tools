#pragma once

#include "DataProcessor.h"
#include "IClient.h"
#include <memory>
#include <cstdint>
#include <string>
#include <thread>
#include <boost/asio.hpp>

class Streamer: public IClient
{
public:
    Streamer();

    void StartAsync() override;
    void HandleEvents() override;
    void Destroy() override;

private:
    const int m_framesDelay;

    std::shared_ptr<boost::asio::io_context> m_ioContext;
    std::shared_ptr<boost::asio::io_context::work> m_work;
    std::shared_ptr<std::thread> m_pipelinePlayThread, m_pipelineSenderThread;

    std::shared_ptr<DataProcessor> m_firstVideoProcessor, m_firstAudioProcessor;
};
