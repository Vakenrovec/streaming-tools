#include "RTPHelper.h"
#include "RtpDefs.h"
#include "Logger.h"
#include "DateTimeUtils.h"
#include <arpa/inet.h>

const uint8_t* RTPHelper::ReadRtppacket(
    const uint8_t* buffer,
    size_t len,
    size_t& payloadLen
) {
    size_t offset = 0;

    if (len < RTP_HEADER_SIZE_BYTES + RTP_EXTENSION_SIZE_BYTES) {
        return nullptr; // buffer too small for rtp
    }

    rtp_header_t* header = (rtp_header_t*) buffer;
    if (header->version != 2) {
        LOG_EX_ERROR("Invalid rtp header version");
        return nullptr;
    }

    this->m_seqNumber = ntohs(header->seq);
    this->m_timestamp = ntohl(header->ts);
    this->m_marker = !!header->m;

    offset += RTP_HEADER_SIZE_BYTES;

    if (header->padding) {
        LOG_EX_WARN("Padding set");
    }

    if (header->csrcCount) {
        // based on rfc: https://www.cl.cam.ac.uk/~jac22/books/mm/book/node159.html
        // the header is extended with the contributing sources, the size of each is 32 bits
        offset += header->csrcCount * RTP_EXTENSION_SIZE_BYTES; 
    }

    if (header->extension) {
        rtp_header_extension_t* extension = (rtp_header_extension_t*) (buffer + offset);
        offset += 4; 
        int extlen = ntohs(extension->length) * RTP_EXTENSION_SIZE_BYTES;
        if(len > (offset + extlen))
            offset += extlen;
    }
    
    if (offset > len) {
        return nullptr;
    } else {
        payloadLen = len - offset;
    }

    // LOG_EX_TRACE(">>>>>>>>>>>>>>>>>>>>> Seq: %d, ts: %lu, pl: %d", m_seqNumber, m_timestamp, payloadLen);

    return (const uint8_t*)(buffer + offset);
}

const bool RTPHelper::ReadTimestampInRtpPacket(
    const uint8_t* buffer,
    size_t len
)
{
    if (len < RTP_HEADER_SIZE_BYTES + RTP_EXTENSION_SIZE_BYTES) {
        return false; // buffer too small for rtp
    }

    rtp_header_t* header = (rtp_header_t*) buffer;
    if (header->version != 2) {
        LOG_EX_ERROR("Invalid rtp header version");
        return false;
    }

    this->m_timestamp = ntohl(header->ts);
    return true;
}

const uint8_t* RTPHelper::ReadFrameInRtpPacket(
    const uint8_t* buffer,
    size_t len,
    size_t& payloadLen
) {
    size_t offset = 0;

    if (len < RTP_HEADER_SIZE_BYTES + RTP_EXTENSION_SIZE_BYTES) {
        return nullptr; // buffer too small for rtp
    }

    rtp_header_t* header = (rtp_header_t*) buffer;
    if (header->version != 2) {
        LOG_EX_ERROR("Invalid rtp header version");
        return nullptr;
    }

    this->m_seqNumber = ntohs(header->seq);
    this->m_timestamp = ntohl(header->ts);
    this->m_marker = !!header->m;

    offset += RTP_HEADER_SIZE_BYTES;

    if (header->csrcCount) {
        // based on rfc: https://www.cl.cam.ac.uk/~jac22/books/mm/book/node159.html
        // the header is extended with the contributing sources, the size of each is 32 bits
        offset += header->csrcCount * RTP_EXTENSION_SIZE_BYTES; 
    }

    if (header->extension) {
        rtp_header_extension_t* extension = (rtp_header_extension_t*) (buffer + offset);
        offset += 4; 
        int extlen = ntohs(extension->length) * RTP_EXTENSION_SIZE_BYTES;
        if(len > (offset + extlen))
            offset += extlen;
    }

    this->m_isKeyFrame = false;
    const uint8_t* tempBuffer = buffer + offset;
    uint8_t vp8pd = *tempBuffer;
    uint8_t xbit = (vp8pd & 0x80);
    uint8_t sbit = (vp8pd & 0x10);
    this->m_sbit = sbit;
    /* Read the Extended control bits octet */
    if(xbit) {
        tempBuffer++; offset++;
        vp8pd = *tempBuffer;
        uint8_t ibit = (vp8pd & 0x80);
        uint8_t lbit = (vp8pd & 0x40);
        uint8_t tbit = (vp8pd & 0x20);
        uint8_t kbit = (vp8pd & 0x10);
        if(ibit) {
            /* Read the PictureID octet */
            tempBuffer++; offset++;
            uint8_t mbit = (vp8pd & 0x80);
            if(mbit) {
                tempBuffer++; offset++;
            }
        }
        if(lbit) {
            /* Read the TL0PICIDX octet */
            tempBuffer++; offset++;
        }
        if(tbit || kbit) {
            /* Read the TID/KEYIDX octet */
            tempBuffer++; offset++;
        }
    }
    tempBuffer++; offset++;	/* Now we're in the payload */
    if(sbit) 
    {
        unsigned long vp8ph = 0;
        std::copy(tempBuffer, tempBuffer + 4, (uint8_t *)&vp8ph);
        vp8ph = ntohl(vp8ph);
        uint8_t pbit = ((vp8ph & 0x01000000) >> 24);
        if (!pbit)
        {
            this->m_isKeyFrame = true;
        }
    }

    if (header->padding) {
        LOG_EX_WARN("Padding set");
        uint8_t paddingLength = *(buffer + len - 1);
        len -= paddingLength;
    }

    if (offset > len) {
        return nullptr;
    } else {
        payloadLen = len - offset;
    }

    LOG_EX_INFO(
        ">>>>>>>>>>>>>>>>>>>>> Seq: " + std::to_string(m_seqNumber) + 
        ", ts = " + std::to_string(m_timestamp) + 
        ", m = " + std::to_string(m_marker) + 
        ", s = " + std::to_string(m_sbit) + 
        ", key = " + std::to_string(m_isKeyFrame) + 
        ", pl: " + std::to_string(payloadLen)
    );

    return (const uint8_t*)(buffer + offset);
}

media_packet_ptr RTPHelper::MakeRtpPacket(std::uint8_t* slice, int size)
{
    rtp_header_t rtpHeader;
    rtpHeader.version = 2;
    rtpHeader.padding = 0;
    rtpHeader.extension = 0;
    rtpHeader.csrcCount = 0;
    rtpHeader.pt = 0; 
    rtpHeader.m = m_marker; 
    rtpHeader.seq = m_seqNumber; 
    rtpHeader.ts = m_timestamp; 
    rtpHeader.ssrc = 0;

    rtp_descriptor_t rtpDescriptor;
    rtpDescriptor.x = 0;
    rtpDescriptor.s = m_sbit; 

    auto packet = std::make_shared<media_packet_t>();
    packet->header.type = PacketType::VIDEO_RTP;
    packet->header.ts = DateTimeUtils::GetCurrentTimeMiliseconds();
    packet->header.size = size + RTP_HEADER_SIZE_BYTES + RTP_DESCRIPTOR_SIZE_BYTES;

    packet->data.resize(packet->header.size);
    std::copy((unsigned char *)&rtpHeader, (unsigned char *)&rtpHeader + RTP_HEADER_SIZE_BYTES, packet->data.data());
    std::copy((unsigned char *)&rtpDescriptor, (unsigned char *)&rtpDescriptor + RTP_DESCRIPTOR_SIZE_BYTES, packet->data.data() + RTP_HEADER_SIZE_BYTES);
    std::copy(slice, slice + size, packet->data.data() + RTP_HEADER_SIZE_BYTES + RTP_DESCRIPTOR_SIZE_BYTES);

    return packet;
}
