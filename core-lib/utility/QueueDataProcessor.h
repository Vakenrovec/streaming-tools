#pragma once

#include "DataProcessor.h"
#include "MediaPacket.h"
#include "UDPPacket.h"
#include <memory>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>

template<typename packet_ptr_t>
class QueueDataProcessor: public DataProcessor
{
public:
    QueueDataProcessor()
    {
    }

    void Init() override
    {
        m_worker = std::thread(std::bind(&QueueDataProcessor<packet_ptr_t>::DoWork, this));
        DataProcessor::Init();
    }

    void Destroy() override
    {
        Close();
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
        while (m_packetQueue.size() == 0)
        {
            m_packetCV.wait(lock);
        }
        const packet_ptr_t pkt = m_packetQueue.front();
        m_packetQueue.pop();
        return pkt;
    }

    void Close(bool handleRemainigPackets = false)
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

    std::thread m_worker;
    std::queue<packet_ptr_t> m_packetQueue;
    std::mutex m_packetMutex;
    std::condition_variable m_packetCV;
};
