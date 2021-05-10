#pragma once

#include "DataProcessor.h"
#include <fstream>

template<typename packet_ptr_t>
class FileSaveRawStreamProcessor: public DataProcessor
{
public:
    FileSaveRawStreamProcessor(const std::string& filename)
    : m_filename(filename)
    {
    }

    void Init() override
    {
        m_file.open(m_filename, std::ios::binary);
        DataProcessor::Init();
    }

    void Destroy() override
    {
        m_file.close();
        DataProcessor::Destroy();
    }

    void Process(const packet_ptr_t& pkt) override
    {
        m_file.write((char*)&pkt->header, sizeof(pkt->header));
        m_file.write((char*)&pkt->data, sizeof(pkt->header.size));
        DataProcessor::Process(pkt);
    }

private:
    std::string m_filename;
    std::ofstream m_file;
};
