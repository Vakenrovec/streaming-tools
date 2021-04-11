#include "RTPVp8DepayDataProcessor.h"
#include "Logger.h"
// #include "FrameUtils.h"

#include <limits>
#include <arpa/inet.h>
#include <vector>

RTPVp8DepayDataProcessor::RTPVp8DepayDataProcessor() 
: m_maxWidth(0)
, m_maxHeight(0)
, m_frameSize(0)
, m_waitForKeyFrameState(true)
, m_frameTimestamp(std::numeric_limits<std::uint32_t>::max())
, m_rtpHelper(new RTPHelper)
{
}

RTPVp8DepayDataProcessor::~RTPVp8DepayDataProcessor() 
{
}

void RTPVp8DepayDataProcessor::Process(const media_packet_ptr& pkt, const data_processor_ptr& prevProcessor) {
    if (pkt->header.type == packet_type_t::VIDEO_RTP) {
        // TODO: add the logic here
        // see RFC: https://tools.ietf.org/html/rfc7741#page-13

        // this object must produce media packets with the type VPXFRAME
        // so it will be something like this:
        // auto newPkt = std::make_shared<media_packet_t>();
        // newPkt->header.size = buffer size;
        // newPkt->header.type = PacketType::VPXFRAME;
        // newPkt->data.resize(newPkt->header.size);
        // ... (copy buffer to newPkt->data)
        //DataProcessor::Process(newPkt);

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
            this->m_framePackets.sort([]( const media_packet_ptr &a, media_packet_ptr &b) {
                rtp_header_t *headerA = (rtp_header_t *)&a->data[0];
                rtp_header_t *headerB = (rtp_header_t *)&b->data[0];
                return ntohs(headerA->seq) < ntohs(headerB->seq); 
            });     
            
            // check the frame for corruption
            bool isFrameValid = !FrameUtils::IsFrameCorrupted(m_framePackets, m_rtpHelper);
            
            // find key frame
            for (const auto& pkt : m_framePackets)
            {
                vpx_frame frame;
                size_t payloadLen = 0;
                auto payload = this->m_rtpHelper->ReadRtppacket(&pkt->data[0], pkt->header.size, payloadLen);
                auto isKeyFrame = VPXUtils::ParseVp8KeyFrame(payload, payloadLen, frame);
                if (isKeyFrame)
                {
                    m_waitForKeyFrameState = false;
                    if (frame.width > m_maxWidth)
                        m_maxWidth = frame.width;
                    if (frame.height > m_maxHeight)
                        m_maxHeight = frame.height;
                }
            } 

            if (isFrameValid && !m_waitForKeyFrameState)
            {                
                // create pkt
                auto framePacket = std::make_shared<media_packet_t>();
                framePacket->header.type = PacketType::VPXFRAME;
                framePacket->header.ts = m_frameTimestamp;
                framePacket->header.size = m_frameSize;
                framePacket->data.resize(framePacket->header.size);

                // copy payload into framePacket->data) 
                int size = 0;          
                for (const auto& pkt : m_framePackets)
                {
                    size_t payloadLen = 0;
                    auto payload = this->m_rtpHelper->ReadFrameInRtpPacket(&pkt->data[0], pkt->header.size, payloadLen);                
                    //framePacket->data.insert(framePacket->data.end(), payload, payload + payloadLen);
                    
                    std::copy(payload, payload + payloadLen, framePacket->data.data() + size);
                    size += payloadLen;

                    LOG_EX_TRACE("--------- isFrameValid: %d, Seq: %d, ts: %lu, pl: %d", 
                        isFrameValid, m_rtpHelper->seqNumber(), m_rtpHelper->timestamp(), payloadLen);
                }   

                DataProcessor::Process(framePacket);                                   
            }
            else
            {
                m_waitForKeyFrameState = true;
            }       
            
            m_frameTimestamp = packetTimestamp;
            m_framePackets.clear();
        }
         
        m_frameSize += payloadLen;
        m_framePackets.push_back(pkt); 
    }

    DataProcessor::Process(pkt, prevProcessor);
}
