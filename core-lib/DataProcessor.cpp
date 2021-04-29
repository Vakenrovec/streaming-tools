#include "DataProcessor.h"

void DataProcessor::Init()
{
    if (this->m_nextProcessor) {
        this->m_nextProcessor->Init();
    }

    this->m_state = State::INITIALIZED;
}

void DataProcessor::Destroy() {
    if (this->m_nextProcessor) {
        this->m_nextProcessor->Destroy();
    }

    this->m_state = State::DESTROYED;
}

void DataProcessor::SetNextProcessor(const data_processor_ptr& processor) {
    this->m_nextProcessor = processor;
}

void DataProcessor::Process(const media_packet_ptr& pkt) {
    if (this->m_nextProcessor) {
        this->m_nextProcessor->Process(pkt);
    }
}

void DataProcessor::Process(const std::list<media_packet_ptr>& pkts) {
    if (this->m_nextProcessor) {
        this->m_nextProcessor->Process(pkts);
    }
}

void DataProcessor::Process(const udp_packet_ptr& pkt) {
    if (this->m_nextProcessor) {
        this->m_nextProcessor->Process(pkt);
    }
}

void DataProcessor::Process(const std::list<udp_packet_ptr>& pkts) {
    if (this->m_nextProcessor) {
        this->m_nextProcessor->Process(pkts);
    }
}
