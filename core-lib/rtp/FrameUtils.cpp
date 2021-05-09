#include "FrameUtils.h"
#include "Logger.h"

bool FrameUtils::IsFrameCorrupted(std::list<udp_packet_ptr>& framePackets, std::shared_ptr<RTPHelper>& rtpHelper)
{
    size_t size = framePackets.size();

    if (!size)
    {
        LOG_EX_WARN("FrameUtils: Invalid size");
        return true;
    }

    size_t payloadLen = 0;
    auto firstPacket = framePackets.front();
    rtpHelper->ReadFrameInRtpPacket(&firstPacket->data[0], firstPacket->header.size, payloadLen);
    if (!rtpHelper->sbit())
    {
        LOG_EX_WARN("FrameUtils: frame doesn't have first packet");
        return true;
    }

    auto lastPacket = framePackets.back();
    rtpHelper->ReadFrameInRtpPacket(&lastPacket->data[0], lastPacket->header.size, payloadLen);
    if (!rtpHelper->marker())
    {
        LOG_EX_WARN("FrameUtils: frame doesn't have last packet, size = " + std::to_string(framePackets.size()));
        return true;
    }

    if (size == 1)
    {
        return false;
    }

    std::list<udp_packet_ptr>::iterator iterator = framePackets.begin();
    udp_packet_ptr previousPacket = *iterator;
    rtpHelper->ReadRtppacket(&previousPacket->data[0], previousPacket->header.size, payloadLen);
    uint32_t seqNumOfPreviousPacket = rtpHelper->seqNumber();
    ++iterator;
    for (; iterator != framePackets.end(); ++iterator)
    {
        udp_packet_ptr currentPacket = *iterator;
        rtpHelper->ReadRtppacket(&currentPacket->data[0], currentPacket->header.size, payloadLen);
        auto seqNumOfCurrentPacket = rtpHelper->seqNumber();
  
        if (seqNumOfCurrentPacket - seqNumOfPreviousPacket != 1)
        {
            LOG_EX_WARN("FrameUtils: there is a hole in a frame");
            return true;
        }

        seqNumOfPreviousPacket = seqNumOfCurrentPacket;
    }

    return false;
}
