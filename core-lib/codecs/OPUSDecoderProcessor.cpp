#include "codecs/OPUSDecoderProcessor.h"
#include "Logger.h"

OPUSDecoderProcessor::OPUSDecoderProcessor()
{

}

void OPUSDecoderProcessor::Init()
{
    DataProcessor::Init();    
}

void OPUSDecoderProcessor::Destroy()
{
    DataProcessor::Destroy();
}

void OPUSDecoderProcessor::Process(const media_packet_ptr& pkt)
{
    if (pkt->header.type == MediaPacketType::OPUS)
    {
        pkt->header.type = MediaPacketType::AUDIO_SAMPLES;
        DataProcessor::Process(pkt);
    }
    else
    {
        LOG_EX_WARN_WITH_CONTEXT("Incorrect packet type: %d", pkt->header.type);
    }
}
