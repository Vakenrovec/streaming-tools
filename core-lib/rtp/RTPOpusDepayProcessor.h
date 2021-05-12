#pragma once

#include "DataProcessor.h"
#include "UDPPacket.h"
#include "RTPHelper.h"
#include <memory>
#include <list>
#include <string>

class RTPOpusDepayProcessor: public DataProcessor
{
public:
    RTPOpusDepayProcessor();
    ~RTPOpusDepayProcessor();

    void Init() override;
    void Destroy() override;

    void Process(const udp_packet_ptr& pkt) override;

private:
    int m_maxWidth, m_maxHeight;
    std::uint64_t m_frameSize;
    std::uint64_t m_frameTimestamp;
    std::list<udp_packet_ptr> m_framePackets;    
    std::shared_ptr<RTPHelper> m_rtpHelper;
};
