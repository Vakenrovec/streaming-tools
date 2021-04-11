#pragma once

#include "MediaPacket.h"
#include "RTPHelper.h"

#include <list>
#include <memory>

class FrameUtils
{
public:
    static bool IsFrameCorrupted(std::list<media_packet_ptr>& framePackets, std::shared_ptr<RTPHelper>& rtpHelper);
};
