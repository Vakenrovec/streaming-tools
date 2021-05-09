#pragma once

#include "DataProcessor.h"

class OPUSEncoderProcessor: public DataProcessor
{
public:
    OPUSEncoderProcessor();

    void Init() override;
    void Destroy() override;

    void Process(const media_packet_ptr& pkt) override;
};
