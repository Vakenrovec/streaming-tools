#pragma once

#include "DataProcessor.h"

class OPUSDecoderProcessor: public DataProcessor
{
public:
    OPUSDecoderProcessor();

    void Init() override;
    void Destroy() override;

    void Process(const media_packet_ptr& pkt) override;
};
