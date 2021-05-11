#include "RTPFragmenterProcessor.h"
#include "Network.h"
#include "RtpDefs.h"
#include "DateTimeUtils.h"
#include "Logger.h"

RTPFragmenterProcessor::RTPFragmenterProcessor(udp_packet_type_t packetType)
: m_rtpHelper(new RTPHelper(packetType))
, m_packetType(packetType)
, m_maxPayloadLength(
    Network::MaxUdpPacketSize - sizeof(media_pkt_header_t) - 
    sizeof(rtp_header_t) - sizeof(rtp_descriptor)
)
{
}

void RTPFragmenterProcessor::Init()
{
    DataProcessor::Init();
}

void RTPFragmenterProcessor::Destroy()
{
    DataProcessor::Destroy();
}

void RTPFragmenterProcessor::Process(const media_packet_ptr& pkt)
{
    udp_packet_ptr udpPacket = nullptr;
    int slices = (pkt->header.size / m_maxPayloadLength) + (!!(pkt->header.size % m_maxPayloadLength));
    // if (m_packetType == udp_packet_type_t::RTP_AUDIO) LOG_EX_INFO_WITH_CONTEXT("slices = " + std::to_string(slices));
    int size = pkt->header.size, pos = 0;
    std::uint32_t seq = 0;
    std::uint64_t ts = pkt->header.ts; // DateTimeUtils::GetCurrentTimeMiliseconds();
    while (size)
    {
        m_rtpHelper->seqNumber(++seq);
        m_rtpHelper->timestamp(ts);
        m_rtpHelper->sbit(seq == 1);
        m_rtpHelper->marker(seq == slices); 
        m_rtpHelper->isKeyFrame(false);
        if (size > m_maxPayloadLength)
        {
            udpPacket = m_rtpHelper->MakeUdpRtpPacket(pkt->data + pos, m_maxPayloadLength, ts);
            pos += m_maxPayloadLength;
            size -= m_maxPayloadLength;
        }
        else
        {
            udpPacket = m_rtpHelper->MakeUdpRtpPacket(pkt->data + pos, size, ts);
            pos += size;
            size -= size;
        }
        DataProcessor::Process(udpPacket);
    }
}
