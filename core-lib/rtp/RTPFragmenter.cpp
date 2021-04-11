#include "RTPFragmenter.h"
#include "udp.h"
#include "Image.h"
#include "RtpDefs.h"
#include "udp.h"
#include "DateTimeUtils.h"

RTPFragmenter::RTPFragmenter()
: m_rtpHelper(new RTPHelper())
, m_maxPayloadLength(
    UDP::MTU - UDP::IPv4HeaderSize - UDP::UDPHeaderSize - 
    MEDIA_PACKET_SIZE_BYTES - RTP_HEADER_SIZE_BYTES - RTP_DESCRIPTOR_SIZE_BYTES
)
{
}

std::list<media_packet_ptr> RTPFragmenter::FragmentRTPFrame(Image& vp8Image)
{
    std::list<media_packet_ptr> packets;
    int slices = (vp8Image.size / m_maxPayloadLength) + (!!(vp8Image.size % m_maxPayloadLength));
    int size = vp8Image.size, pos = 0;
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
            media_packet_ptr packet = m_rtpHelper->MakeRtpPacket(vp8Image.data + pos, m_maxPayloadLength);
            packets.push_back(packet);
            pos += m_maxPayloadLength;
            size -= m_maxPayloadLength;
        }
        else
        {
            media_packet_ptr packet = m_rtpHelper->MakeRtpPacket(vp8Image.data + pos, size);
            packets.push_back(packet);
            pos += size;
            size -= size;
        }
    }

    return packets;
}

Image RTPFragmenter::DefragmentRTPPackets(std::list<media_packet_ptr>& packets)
{
    int size = 0;          
    for (const auto& pkt : packets)
    {
        size += pkt->header.size;
    }

    Image image;
    image.size = size;
    image.data = new std::uint8_t[size];
    int pos = 0;
    size_t payloadLen = 0;
    for (const auto& pkt : packets)
    {
        auto payload = m_rtpHelper->ReadFrameInRtpPacket(&pkt->data[0], pkt->header.size, payloadLen);        
        std::copy(payload, payload + payloadLen, image.data + pos);
        pos += payloadLen;
    }

    return image;
}