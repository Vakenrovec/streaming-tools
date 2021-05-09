#pragma once

#include "DataProcessor.h"
#include "MediaPacket.h"
#include "UDPPacket.h"
#include "RTPHelper.h"
#include <cstdint>
#include <list>
#include <memory>

class RTPDefragmenterProcessor: public DataProcessor
{
public:
    RTPDefragmenterProcessor(media_packet_type_t packetType = media_packet_type_t::UNKNOWN);

    void Init() override;
    void Destroy() override;

    void Process(const std::list<udp_packet_ptr>& pkts) override;

private:
    media_packet_ptr DefragmentRTPPackets(const std::list<udp_packet_ptr>& packets);
    media_packet_ptr DefragmentRTPPackets(const std::list<udp_packet_ptr>& packets, std::uint32_t payloadSize);

    std::shared_ptr<RTPHelper> m_rtpHelper;
    const int m_maxPayloadLength;

    media_packet_type_t m_packetType;
};
