#pragma once

#include <Network.h>
#include <cstdint>
#include <memory>

enum class udp_packet_type_t: std::uint8_t {
    UNKNOWN = 0,
    RTP_VIDEO = 1,
    RTP_AUDIO = 2,
};

#pragma pack(push, 1)

typedef struct {
    udp_packet_type_t type;          // payload type 
    std::uint64_t ts;                // timestamp
    std::uint32_t size;              // the size of the payload data 
} udp_pkt_header_t;

#pragma pack(pop)

constexpr inline std::uint32_t UDPPacketDataSize = Network::MaxUdpPacketSize - sizeof(udp_pkt_header_t);

typedef struct {
    udp_pkt_header_t header;
    std::uint8_t data[UDPPacketDataSize];               // rtp packet
} udp_packet_t;

using udp_packet_ptr = std::shared_ptr<udp_packet_t>;
