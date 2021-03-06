#pragma once

#include "DataProcessor.h"
#include "Logger.h"
#include "FileUtils.h"
#include <fstream>

template<typename packet_ptr_t>
class FileSaveRawStreamProcessor: public DataProcessor
{
public:
    FileSaveRawStreamProcessor(const std::string& dir, const std::string& filename)
    : m_filename(filename)
    , m_dir(dir)
    , m_fullPath(FileUtils::CombinePath(dir, filename))
    {
    }

    void Init() override
    {
        FileUtils::CreateDirs(boost::filesystem::path(m_dir));
        m_file.open(m_fullPath, std::ios::binary | std::ios::trunc);
        if (!m_file)
        {
            LOG_EX_ERROR_WITH_CONTEXT("Couldn't open file %s", m_fullPath.c_str());
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
        m_file.write((char*)&pkt->header, sizeof(pkt->header));
        m_file.write((char*)&pkt->data, pkt->header.size);
        DataProcessor::Process(pkt);
    }

private:
    std::string m_dir, m_filename, m_fullPath;
    std::ofstream m_file;
};
