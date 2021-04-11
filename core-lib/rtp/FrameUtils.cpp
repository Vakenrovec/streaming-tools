#include "FrameUtils.h"

bool FrameUtils::IsFrameCorrupted(std::list<media_packet_ptr>& framePackets, std::shared_ptr<RTPHelper>& rtpHelper)
{
    size_t size = framePackets.size();

    if (!size)
    {
        return true;
    }

    size_t payloadLen = 0;
    auto firstPacket = framePackets.front();
    rtpHelper->ReadFrameInRtpPacket(&firstPacket->data[0], firstPacket->header.size, payloadLen);
    if (!rtpHelper->sbit())
    {
        return true;
    }

    auto lastPacket = framePackets.back();
    rtpHelper->ReadFrameInRtpPacket(&lastPacket->data[0], lastPacket->header.size, payloadLen);
    if (!rtpHelper->marker())
    {
        return true;
    }

    if (size == 1)
    {
        return false;
    }

    std::list<media_packet_ptr>::iterator iterator = framePackets.begin();
    media_packet_ptr previousPacket = *iterator;
    rtpHelper->ReadRtppacket(&previousPacket->data[0], previousPacket->header.size, payloadLen);
    uint32_t seqNumOfPreviousPacket = rtpHelper->seqNumber();
    ++iterator;
    for (; iterator != framePackets.end(); ++iterator)
    {        
        media_packet_ptr currentPacket = *iterator;
        rtpHelper->ReadRtppacket(&currentPacket->data[0], currentPacket->header.size, payloadLen);
        auto seqNumOfCurrentPacket = rtpHelper->seqNumber();
  
        if (seqNumOfCurrentPacket - seqNumOfPreviousPacket != 1)
        {
            return true;
        }

        seqNumOfPreviousPacket = seqNumOfCurrentPacket;
    }

    return false;
}
