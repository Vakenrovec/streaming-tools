#include "RTPVp8Depay.h"
#include "RtpDefs.h"
#include "Logger.h"
#include "FrameUtils.h"
#include "RTPFragmenter.h"
#include "Image.h"
#include <limits>
#include <arpa/inet.h>
#include <vector>
#include <fstream>

RTPVp8Depay::RTPVp8Depay() 
: m_maxWidth(0)
, m_maxHeight(0)
, m_frameSize(0)
, m_waitForKeyFrameState(true)
, m_frameTimestamp(std::numeric_limits<std::uint32_t>::max())
, m_rtpHelper(new RTPHelper)
, m_videoDisplay()
{
    m_vp8codec.InitDecodeContext();
}

RTPVp8Depay::~RTPVp8Depay() 
{
}

void RTPVp8Depay::Process(const media_packet_ptr& pkt) {
    if (pkt->header.type == packet_type_t::VIDEO_RTP) {

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
                size_t payloadLen = 0;
                auto payload = this->m_rtpHelper->ReadFrameInRtpPacket(&pkt->data[0], pkt->header.size, payloadLen);
                auto isKeyFrame = this->m_rtpHelper->isKeyFrame();
                if (isKeyFrame)
                {
                    m_waitForKeyFrameState = false;
                }
            } 

            if (isFrameValid && !m_waitForKeyFrameState)
            {                
                // create pkt
                // auto framePacket = std::make_shared<media_packet_t>();
                // framePacket->header.type = PacketType::VPXFRAME;
                // framePacket->header.ts = m_frameTimestamp;
                // framePacket->header.size = m_frameSize;
                // framePacket->data.resize(framePacket->header.size);

                // copy payload into framePacket->data) 
                int size = 0;          
                for (const auto& pkt : m_framePackets)
                {
                    // size_t payloadLen = 0;
                    // auto payload = this->m_rtpHelper->ReadFrameInRtpPacket(&pkt->data[0], pkt->header.size, payloadLen);
                    // std::copy(payload, payload + payloadLen, framePacket->data.data() + size);
                    // size += payloadLen;

                    // LOG_EX_TRACE("--------- isFrameValid: %d, Seq: %d, ts: %lu, pl: %d", 
                    //     isFrameValid, m_rtpHelper->seqNumber(), m_rtpHelper->timestamp(), payloadLen);
                }   

                RTPFragmenter rtpFragmenter;
                Image imageVp8 = rtpFragmenter.DefragmentRTPPackets(m_framePackets);
                Image yv12Image;
                std::ofstream outFile;
                m_vp8codec.Decode(imageVp8, yv12Image);
                static int i = 0; i++;
                outFile.open("receiverFolder/receiver_output" + std::to_string(i) + ".yv12", std::ios::binary | std::ios::trunc);
                outFile.write((char*)yv12Image.data, yv12Image.size);
                outFile.close();
                m_videoDisplay.Display(yv12Image);
                delete[] yv12Image.data;
                // LOG_EX_INFO("!!!\n\n\n\n!!!");                                  
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

    // DataProcessor::Process(pkt, prevProcessor);
}
