#pragma once

#include "UDPPacket.h"
#include <cstdint>
#include <string>

class RTPHelper {
public:
    RTPHelper() = default;

    /**
     * @returns nullptr in case of error, otherwise returns pointer where payload starts 
     * and fills  payloadLen with actual payload len in the original buffer
     */
    const std::uint8_t* ReadRtppacket(
        const std::uint8_t* buffer,
        size_t len,
        size_t& payloadLen
    );
    
    const bool ReadTimestampInRtpPacket(
        const uint8_t* buffer,
        size_t len
    );

    const uint8_t* ReadFrameInRtpPacket(
        const uint8_t* buffer,
        size_t len,
        size_t& payloadLen
    );


    udp_packet_ptr MakeUdpRtpPacket(std::uint8_t* slice, int size);
    udp_packet_ptr MakeUdpRtpPacket(std::uint8_t* slice, int size, std::uint64_t ts);

    inline std::uint32_t timestamp() const { return this->m_timestamp; }
    inline std::uint16_t seqNumber() const { return this->m_seqNumber; } 
    inline bool marker() const { return this->m_marker; }
    inline bool sbit() const { return this->m_sbit; }
    inline bool isKeyFrame() const { return this->m_isKeyFrame; }

    inline void timestamp(std::uint32_t ts)  { this->m_timestamp = ts; }
    inline void seqNumber(std::uint16_t seq) { this->m_seqNumber = seq; } 
    inline void marker(bool m) { this->m_marker = m; }
    inline void sbit(bool s) { this->m_sbit = s; }
    inline void isKeyFrame(bool isKey) { this->m_isKeyFrame = isKey; }

private:
    std::uint32_t m_timestamp = 0;
    std::uint16_t m_seqNumber = 0;
    bool m_marker = false;
    bool m_sbit = false;
    bool m_isKeyFrame = false;
};
