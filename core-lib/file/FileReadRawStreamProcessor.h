#pragma once

#include "PlayableDataProcessor.h"
#include "Logger.h"
#include "FileUtils.h"
#include <fstream>

template<typename packet_ptr_t>
class FileReadRawStreamProcessor: public PlayableDataProcessor
{
public:
    FileReadRawStreamProcessor(const std::string& dir, const std::string& filename)
    : m_filename(filename)
    , m_dir(dir)
    , m_fullPath(FileUtils::CombinePath(dir, filename))
    {
    }

    void Init() override
    {
        m_file.open(m_fullPath, std::ios::binary);
        if (!m_file)
        {
            LOG_EX_ERROR_WITH_CONTEXT("Couldn't open file %s", m_fullPath.c_str());
            return;
        }
        DataProcessor::Init();
    }

    void Destroy() override
    {
        m_file.close();
        DataProcessor::Destroy();
    }

    void Process(const packet_ptr_t& pkt) override
    {
        m_file.read((char*)&pkt->header, sizeof(pkt->header));
        if (!m_file){
            return;
        }
        m_file.read((char*)&pkt->data, pkt->header.size);
        DataProcessor::Process(pkt);
    }

    int Play(int count = 0) override
    {
        count = 0;
        while (!m_file.eof() && !stop)
        {
            const auto pkt = std::make_shared<typename packet_ptr_t::element_type>();
            Process(pkt);
            count++;
        }
        return count;
    }

private:
    std::string m_dir, m_filename, m_fullPath;
    std::ifstream m_file;
};
