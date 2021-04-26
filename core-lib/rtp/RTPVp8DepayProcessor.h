#pragma once

#include "DataProcessor.h"
#include "MediaPacket.h"
#include "RTPHelper.h"
#include <memory>
#include <list>

class RTPVp8DepayProcessor: public DataProcessor
{
public:
    RTPVp8DepayProcessor();
    ~RTPVp8DepayProcessor();

    void Init() override;
    void Destroy() override;

    void Process(const media_packet_ptr& pkt) override;

private:
    int m_maxWidth, m_maxHeight;
    bool m_waitForKeyFrameState;
    std::uint32_t m_frameSize;
    std::uint32_t m_frameTimestamp;
    std::list<media_packet_ptr> m_framePackets;    
    std::shared_ptr<RTPHelper> m_rtpHelper;
};
