#pragma once

#include "MediaPacket.h"
#include <memory>
#include <list>

class DataProcessor: public std::enable_shared_from_this<DataProcessor>
{
public:
    enum class State {
        INITIALIZED,
        DESTROYED,
        STOPPED,        
        ERROR
    };

    using data_processor_ptr = std::shared_ptr<DataProcessor>;

public:
    DataProcessor() = default;
    virtual ~DataProcessor() = default;

    virtual void Init();
    virtual void Destroy();
    
    inline State GetState() const { return this->m_state; }
    void SetNextProcessor(const data_processor_ptr& processor);

    virtual void Process(const media_packet_ptr& pkt);
    virtual void Process(const std::list<media_packet_ptr>& pkts);

protected:
    data_processor_ptr m_nextProcessor = nullptr;
    State m_state = State::STOPPED;
};
