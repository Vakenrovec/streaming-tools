#pragma once

#include "UDPPacket.h"
#include "RTPHelper.h"

#include <list>
#include <memory>

class FrameUtils
{
public:
    static bool IsFrameCorrupted(std::list<udp_packet_ptr>& framePackets, std::shared_ptr<RTPHelper>& rtpHelper);
};
