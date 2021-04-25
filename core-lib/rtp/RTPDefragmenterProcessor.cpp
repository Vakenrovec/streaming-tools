#include "RTPDefragmenterProcessor.h"
#include "Network.h"
#include "Image.h"
#include "RtpDefs.h"
#include "DateTimeUtils.h"

RTPDefragmenterProcessor::RTPDefragmenterProcessor()
: m_rtpHelper(new RTPHelper())
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

void RTPDefragmenterProcessor::Process(const std::list<media_packet_ptr>& pkts)
{
    auto pkt = DefragmentRTPPackets(pkts);
    DataProcessor::Process(pkt);
}

media_packet_ptr RTPDefragmenterProcessor::DefragmentRTPPackets(const std::list<media_packet_ptr>& packets)
{
    auto pkt = std::make_shared<media_packet_t>();
    pkt->header.type = packet_type_t::VP8;
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

media_packet_ptr RTPDefragmenterProcessor::DefragmentRTPPackets(const std::list<media_packet_ptr>& packets, std::uint32_t payloadSize)
{
    auto pkt = std::make_shared<media_packet_t>();
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
