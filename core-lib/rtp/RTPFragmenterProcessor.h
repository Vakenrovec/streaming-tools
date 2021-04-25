#pragma once

#include "DataProcessor.h"
#include "MediaPacket.h"
#include "RTPHelper.h"
#include <cstdint>
#include <list>
#include <memory>

class RTPFragmenterProcessor: public DataProcessor
{
public:
    RTPFragmenterProcessor();

    void Init() override;
    void Destroy() override;

    void Process(const media_packet_ptr& pkt) override;

private:
    std::list<media_packet_ptr> FragmentRTPFrame(const media_packet_ptr& pkt);

    std::shared_ptr<RTPHelper> m_rtpHelper;
    const int m_maxPayloadLength;
};
