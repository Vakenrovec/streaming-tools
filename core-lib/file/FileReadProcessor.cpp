#include "FileReadProcessor.h"
#include "FileUtils.h"
#include <cstdint>
#include <vector>

FileReadProcessor::FileReadProcessor(std::string& name)
: m_name(name)
, m_counter(0)
{
}

void FileReadProcessor::Init()
{
    DataProcessor::Init();
}

void FileReadProcessor::Destroy()
{
    DataProcessor::Destroy();
}

int FileReadProcessor::Play(int wanted)
{
    for (int i = 0; i < wanted; i++)
    {
        std::string name = m_name + " (" + std::to_string(m_counter++) + ")";
        std::vector<std::uint8_t> buffer;
        if (!FileUtils::ReadFile(name, buffer))
        {
            break;
        }
        auto pkt = std::make_shared<media_packet_t>();
        pkt->header.type = PacketType::UNKNOWN;
        pkt->header.size = buffer.size();
        memcpy(pkt->data, buffer.data(), buffer.size());
        DataProcessor::Process(pkt);
    }
    return m_counter;
}

void FileReadProcessor::Process(const media_packet_ptr& pkt)
{
    DataProcessor::Process(pkt);
}
