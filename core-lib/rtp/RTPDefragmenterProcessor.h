#pragma once

#include "DataProcessor.h"
#include "MediaPacket.h"
#include "RTPHelper.h"
#include <cstdint>
#include <list>
#include <memory>

class RTPDefragmenterProcessor: public DataProcessor
{
public:
    RTPDefragmenterProcessor();

    void Init() override;
    void Destroy() override;

    void Process(const std::list<media_packet_ptr>& pkts) override;

private:
    media_packet_ptr DefragmentRTPPackets(const std::list<media_packet_ptr>& packets);
    media_packet_ptr DefragmentRTPPackets(const std::list<media_packet_ptr>& packets, std::uint32_t payloadSize);

    std::shared_ptr<RTPHelper> m_rtpHelper;
    const int m_maxPayloadLength;
};
