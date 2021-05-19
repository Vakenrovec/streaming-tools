#pragma once

#include "DataProcessor.h"
#include "IClient.h"
#include <memory>
#include <cstdint>
#include <string>
#include <thread>
#include <boost/asio.hpp>

class Receiver: public IClient
{
public:
    Receiver();

    void StartAsync() override;
    void HandleEvents() override;
    void Destroy() override;

private:
    std::shared_ptr<boost::asio::io_context> m_ioContext;
    std::shared_ptr<boost::asio::io_context::work> m_work;
    std::shared_ptr<std::thread> m_pipelineReceiverThread;

    std::shared_ptr<DataProcessor> m_firstProcessor;
};
