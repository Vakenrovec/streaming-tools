#include "FileSaveProcessor.h"
#include "FileUtils.h"

FileSaveProcessor::FileSaveProcessor(std::string& name)
: m_name(name)
, m_counter(0)
{
}

void FileSaveProcessor::Init()
{
    DataProcessor::Init();
}

void FileSaveProcessor::Destroy()
{
    DataProcessor::Destroy();
}

void FileSaveProcessor::Process(const media_packet_ptr& pkt)
{
    std::string name = m_name + " (" + std::to_string(m_counter++) + ")";
    FileUtils::WriteFile(name, (char*)pkt->data, pkt->header.size);
    DataProcessor::Process(pkt);
}
