#pragma once

#include "MediaPacket.h"
#include "RTPHelper.h"
#include <cstdint>
#include <list>
#include <memory>

struct Image;

class RTPFragmenter
{
public:
    RTPFragmenter();
    std::list<media_packet_ptr> FragmentRTPFrame(Image& vp8Image);
    Image DefragmentRTPPackets(std::list<media_packet_ptr>& packets);
    Image DefragmentRTPPackets(std::list<media_packet_ptr>& packets, std::uint32_t payloadSize);

private:
    std::shared_ptr<RTPHelper> m_rtpHelper;
    const int m_maxPayloadLength;
};
