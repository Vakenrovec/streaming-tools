#include "RTPVp8DepayProcessor.h"
#include "RtpDefs.h"
#include "Logger.h"
#include "FrameUtils.h"
#include <cstdint>
#include <limits>
#include <arpa/inet.h>

RTPVp8DepayProcessor::RTPVp8DepayProcessor() 
: m_maxWidth(0)
, m_maxHeight(0)
, m_frameSize(0)
, m_waitForKeyFrameState(true)
, m_frameTimestamp(std::numeric_limits<std::uint32_t>::max())
, m_rtpHelper(new RTPHelper)
{
}

RTPVp8DepayProcessor::~RTPVp8DepayProcessor() 
{
}

void RTPVp8DepayProcessor::Init()
{
    DataProcessor::Init();
}

void RTPVp8DepayProcessor::Destroy()
{
    DataProcessor::Destroy();
}

void RTPVp8DepayProcessor::Process(const udp_packet_ptr& pkt) {
    // if (pkt->header.type == udp_packet_type_t::RTP_VIDEO || pkt->header.type == udp_packet_type_t::RTP_AUDIO) {

        size_t payloadLen = 0;
        if (!this->m_rtpHelper->ReadFrameInRtpPacket(&pkt->data[0], pkt->header.size, payloadLen))
        {
            return;
        }

        auto packetTimestamp = this->m_rtpHelper->timestamp();

        // we got packet at the first time
        if (m_frameTimestamp == std::numeric_limits<std::uint32_t>::max())
        {
            m_frameTimestamp = packetTimestamp;
        }

        // we got outdated packet
        if (packetTimestamp < m_frameTimestamp)
        {
            return;
        }

        // we got newer packet than we had
        if (m_frameTimestamp != packetTimestamp)
        {
            // sort packets in a frame
            this->m_framePackets.sort([]( const udp_packet_ptr &a, udp_packet_ptr &b) {
                rtp_header_t *headerA = (rtp_header_t *)&a->data[0];
                rtp_header_t *headerB = (rtp_header_t *)&b->data[0];
                return ntohs(headerA->seq) < ntohs(headerB->seq); 
            });     

            // check the frame for corruption
            bool isFrameValid = !FrameUtils::IsFrameCorrupted(m_framePackets, m_rtpHelper);
            
            // find key frame
            for (const auto& packet : m_framePackets)
            {
                size_t payloadLen = 0;
                auto payload = this->m_rtpHelper->ReadFrameInRtpPacket(&packet->data[0], packet->header.size, payloadLen);
                auto isKeyFrame = this->m_rtpHelper->isKeyFrame();
                if (isKeyFrame)
                {
                    m_waitForKeyFrameState = false;
                }
            } 

            if (isFrameValid && !m_waitForKeyFrameState)
            {
                // copy payload into framePacket->data)                
                DataProcessor::Process(m_framePackets);                               
            }
            else
            {
                LOG_EX_INFO("Frame is not valid");
                m_waitForKeyFrameState = true;
            }       
            
            m_frameTimestamp = packetTimestamp;
            m_framePackets.clear();
        }
         
        m_frameSize += payloadLen;
        m_framePackets.push_back(pkt); 
    // }

    // DataProcessor::Process(pkt, prevProcessor);
}
