#pragma once

#include "RTPHelper.h"
#include "MediaPacket.h"

#include <memory>
#include <list>

class RTPVp8DepayDataProcessor
{
public:
    RTPVp8DepayDataProcessor();
    ~RTPVp8DepayDataProcessor();

    void Process(const media_packet_ptr& pkt, const data_processor_ptr& prevProcessor = data_processor_ptr()) override;

private:
    int m_maxWidth, m_maxHeight;
    bool m_waitForKeyFrameState;
    std::uint32_t m_frameSize;
    std::uint32_t m_frameTimestamp;    
    std::list<media_packet_ptr> m_framePackets;    
    std::shared_ptr<RTPHelper> m_rtpHelper;
};
