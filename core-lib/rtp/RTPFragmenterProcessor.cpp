#include "RTPFragmenterProcessor.h"
#include "Network.h"
#include "Image.h"
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
    std::list<media_packet_ptr> packets = std::move(FragmentRTPFrame(pkt));
    DataProcessor::Process(packets);
}

std::list<media_packet_ptr> RTPFragmenterProcessor::FragmentRTPFrame(const media_packet_ptr& pkt)
{
    std::list<media_packet_ptr> packets;
    int slices = (pkt->header.size / m_maxPayloadLength) + (!!(pkt->header.size % m_maxPayloadLength));
    int size = pkt->header.size, pos = 0;
    std::uint32_t seq = 0;
    while (size)
    {       
        m_rtpHelper->seqNumber(++seq);
        m_rtpHelper->timestamp(DateTimeUtils::GetCurrentTimeMiliseconds());
        m_rtpHelper->sbit(seq == 1 ? true : false);
        m_rtpHelper->marker(seq == slices ? true : false); 
        m_rtpHelper->isKeyFrame(false);
        if (size > m_maxPayloadLength)
        {
            media_packet_ptr packet = m_rtpHelper->MakeRtpPacket(pkt->data + pos, m_maxPayloadLength);
            packets.push_back(packet);
            pos += m_maxPayloadLength;
            size -= m_maxPayloadLength;
        }
        else
        {
            media_packet_ptr packet = m_rtpHelper->MakeRtpPacket(pkt->data + pos, size);
            packets.push_back(packet);
            pos += size;
            size -= size;
        }
    }

    return packets;
}
