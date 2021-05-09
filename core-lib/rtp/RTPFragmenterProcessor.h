#pragma once

#include "DataProcessor.h"
#include "MediaPacket.h"
#include "UDPPacket.h"
#include "RTPHelper.h"
#include <cstdint>
#include <list>
#include <memory>

class RTPFragmenterProcessor: public DataProcessor
{
public:
    RTPFragmenterProcessor(udp_packet_type_t packetType = udp_packet_type_t::UNKNOWN);

    void Init() override;
    void Destroy() override;

    void Process(const media_packet_ptr& pkt) override;

private:
    std::shared_ptr<RTPHelper> m_rtpHelper;
    const int m_maxPayloadLength;
    udp_packet_type_t m_packetType;
};
