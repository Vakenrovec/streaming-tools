#pragma once

#include "DataProcessor.h"
#include "MediaPacket.h"
#include "UDPPacket.h"
#include <memory>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>

template<typename packet_ptr_t>
class QueueDataProcessor: public DataProcessor
{
public:
    QueueDataProcessor(int delay = 0, bool handleRemainigPackets = true)
    : m_handleRemainigPackets(handleRemainigPackets)
    , m_delay(delay)
    {
    }

    void Init() override
    {
        m_worker = std::thread(std::bind(&QueueDataProcessor<packet_ptr_t>::DoWork, this));
        DataProcessor::Init();
    }

    void Destroy() override
    {
        m_state = State::STOPPING;
        Close(m_handleRemainigPackets);
        DataProcessor::Destroy();
    }
    
    void Process(const packet_ptr_t& pkt) override
    {
        Enqueue(pkt);
    }

    std::shared_ptr<std::thread> worker;

private:
    void DoWork()
    {
        while (true)
        {
            packet_ptr_t pkt = Dequeue();
            if (!pkt)
            {
                break;
            }
            DataProcessor::Process(pkt);
        }
    }

    void Enqueue(const packet_ptr_t& pkt)
    {
        std::unique_lock<std::mutex> lock(m_packetMutex);
        m_packetQueue.push(pkt);
        m_packetCV.notify_one();
    }

    packet_ptr_t Dequeue()
    {
        std::unique_lock<std::mutex> lock(m_packetMutex);
        while (m_packetQueue.size() <= m_delay && m_state != State::STOPPING)
        {
            m_packetCV.wait(lock);
        }
        const packet_ptr_t pkt = m_packetQueue.front();
        m_packetQueue.pop();
        return pkt;
    }

    void Close(bool handleRemainigPackets)
    {
        {
            std::unique_lock<std::mutex> lock(m_packetMutex);
            if (!handleRemainigPackets)
            {
                m_packetQueue = {};
            }
            m_packetQueue.push(nullptr);
            m_packetCV.notify_all();
        }
        m_worker.join();
    }

private:
    std::thread m_worker;
    std::queue<packet_ptr_t> m_packetQueue;
    std::mutex m_packetMutex;
    std::condition_variable m_packetCV;

    int m_delay;
    bool m_handleRemainigPackets;
};
