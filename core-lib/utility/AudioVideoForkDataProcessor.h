#pragma once

#include "utility/ForkDataProcessor.h"
#include "UDPPacket.h"
#include <cstdint>

template<std::uint8_t Nm>
class AudioVideoForkDataProcessor: public ForkDataProcessor<Nm>
{
public:
    void Process(const udp_packet_ptr& pkt) override
    {
        switch (pkt->header.type)
        {
        case udp_packet_type_t::AUDIO:
            this->m_processors[0]->Process(pkt);
            break;
        case udp_packet_type_t::RTP:
            this->m_processors[1]->Process(pkt);
            break;
        default:
            break;
        }
    }
};
