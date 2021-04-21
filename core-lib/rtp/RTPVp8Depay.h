#pragma once

#include "RTPHelper.h"
#include "MediaPacket.h"
#include "codecs/vp8.h"
#include "video/VideoDisplay.h"
#include <memory>
#include <list>

class RTPVp8Depay
{
public:
    RTPVp8Depay();
    ~RTPVp8Depay();

    void Process(const media_packet_ptr& pkt);

private:
    int m_maxWidth, m_maxHeight;
    bool m_waitForKeyFrameState;
    std::uint32_t m_frameSize;
    std::uint32_t m_frameTimestamp;
    std::list<media_packet_ptr> m_framePackets;    
    std::shared_ptr<RTPHelper> m_rtpHelper;
    VP8Codec m_vp8codec;
    VideoDisplay m_videoDisplay;
};
