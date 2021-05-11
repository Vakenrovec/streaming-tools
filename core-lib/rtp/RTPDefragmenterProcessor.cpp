#include "RTPDefragmenterProcessor.h"
#include "Network.h"
#include "RtpDefs.h"
#include "DateTimeUtils.h"

RTPDefragmenterProcessor::RTPDefragmenterProcessor(media_packet_type_t packetType)
: m_rtpHelper(new RTPHelper())
, m_packetType(packetType)
, m_maxPayloadLength(
    Network::MaxUdpPacketSize - sizeof(media_pkt_header_t) - 
    sizeof(rtp_header_t) - sizeof(rtp_descriptor)
)
{
}

void RTPDefragmenterProcessor::Init()
{
    DataProcessor::Init();
}

void RTPDefragmenterProcessor::Destroy()
{
    DataProcessor::Destroy();
}

void RTPDefragmenterProcessor::Process(const std::list<udp_packet_ptr>& pkts)
{
    auto pkt = DefragmentRTPPackets(pkts);
    DataProcessor::Process(pkt);
}

media_packet_ptr RTPDefragmenterProcessor::DefragmentRTPPackets(const std::list<udp_packet_ptr>& packets)
{
    auto pkt = std::make_shared<media_packet_t>();
    pkt->header.type = m_packetType;
    pkt->header.ts = packets.front()->header.ts;
    int pos = 0;
    size_t payloadLen = 0;
    for (const auto& packet : packets)
    {
        auto payload = m_rtpHelper->ReadFrameInRtpPacket(&packet->data[0], packet->header.size, payloadLen);        
        std::copy(payload, payload + payloadLen, pkt->data + pos);
        pos += payloadLen;
    }
    pkt->header.size = pos;

    return pkt;
}

media_packet_ptr RTPDefragmenterProcessor::DefragmentRTPPackets(const std::list<udp_packet_ptr>& packets, std::uint32_t payloadSize)
{
    auto pkt = std::make_shared<media_packet_t>();
    pkt->header.type = m_packetType;
    pkt->header.ts = packets.front()->header.ts;
    pkt->header.size = payloadSize;
    int pos = 0;
    size_t payloadLen = 0;
    for (const auto& packet : packets)
    {
        auto payload = m_rtpHelper->ReadFrameInRtpPacket(&packet->data[0], packet->header.size, payloadLen);        
        std::copy(payload, payload + payloadLen, pkt->data + pos);
        pos += payloadLen;
    }

    return pkt;
}
