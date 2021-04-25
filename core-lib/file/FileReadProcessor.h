#pragma once

#include "DataProcessor.h"
#include "MediaPacket.h"
#include <string>

class FileReadProcessor: public DataProcessor
{
public:
    FileReadProcessor(std::string& name);

    void Init() override;
    void Destroy() override;

    int Play(int wanted);
    void Process(const media_packet_ptr& pkt) override;

private:
    std::string m_name;
    int m_counter;
};
