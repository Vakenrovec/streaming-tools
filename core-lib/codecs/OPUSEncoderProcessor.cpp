#include "codecs/OPUSEncoderProcessor.h"
#include "Logger.h"

OPUSEncoderProcessor::OPUSEncoderProcessor()
{

}

void OPUSEncoderProcessor::Init()
{
    DataProcessor::Init();    
}

void OPUSEncoderProcessor::Destroy()
{
    DataProcessor::Destroy();
}

void OPUSEncoderProcessor::Process(const media_packet_ptr& pkt)
{
    if (pkt->header.type == MediaPacketType::AUDIO_SAMPLES)
    {
        pkt->header.type = MediaPacketType::OPUS;
        DataProcessor::Process(pkt);
    }
    else
    {
        LOG_EX_WARN_WITH_CONTEXT("Incorrect packet type: %d", pkt->header.type);
    }
}
