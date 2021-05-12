#pragma once 

#include "DataProcessor.h"
#include <thread>
#include <chrono>
#include <cstdint>

class DelayDataProcessor: public DataProcessor
{
public:
    void Process(const media_packet_ptr& pkt) override
    {
        if (m_prevTs != 0)
        {
            m_delay = pkt->header.ts - m_prevTs;
            std::this_thread::sleep_for(std::chrono::nanoseconds(m_delay));
        }
        m_prevTs = pkt->header.ts;
        DataProcessor::Process(pkt);
    }

private:
    uint64_t m_prevTs = 0, m_delay = 0;
};
