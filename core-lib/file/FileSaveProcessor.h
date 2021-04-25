#pragma once

#include "DataProcessor.h"
#include "MediaPacket.h"
#include <string>

class FileSaveProcessor: public DataProcessor
{
public:
    FileSaveProcessor(std::string& name);

    void Init() override;
    void Destroy() override;

    void Process(const media_packet_ptr& pkt) override;

private:
    std::string m_name;
    int m_counter;
};
