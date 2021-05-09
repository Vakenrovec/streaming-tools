#pragma once

#include "utility/ForkDataProcessor.h"
#include "UDPPacket.h"
#include <cstdint>
#include "Logger.h"

template<std::uint8_t Nm>
class AudioVideoForkDataProcessor: public ForkDataProcessor<Nm>
{
public:
    void Process(const udp_packet_ptr& pkt) override
    {
        switch (pkt->header.type)
        {
        case udp_packet_type_t::RTP_AUDIO:
            this->m_processors[0]->Process(pkt);
            break;
        case udp_packet_type_t::RTP_VIDEO:
            this->m_processors[1]->Process(pkt);
            break;
        default:
            LOG_EX_WARN_WITH_CONTEXT("Unknown packet type");
            break;
        }
    }
};
