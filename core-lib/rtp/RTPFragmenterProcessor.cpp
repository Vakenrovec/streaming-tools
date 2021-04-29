#include "RTPFragmenterProcessor.h"
#include "UDPPacket.h"
#include "Network.h"
#include "RtpDefs.h"
#include "DateTimeUtils.h"

RTPFragmenterProcessor::RTPFragmenterProcessor()
: m_rtpHelper(new RTPHelper())
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
    int size = pkt->header.size, pos = 0;
    std::uint32_t seq = 0;
    std::uint64_t ts = DateTimeUtils::GetCurrentTimeMiliseconds();
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
