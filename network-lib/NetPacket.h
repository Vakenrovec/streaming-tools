#pragma once

#include <cstdint>
#include <string>
#include <memory>

typedef enum NetPacketType: std::uint8_t {
    CREATE = 1,
    DESTROY = 2,
    CONNECT = 3,
    DISCONNECT = 4,
} net_packet_type_t;

#pragma pack(push, 1)

typedef struct {
    net_packet_type_t type;          // payload type 
    std::uint32_t id;                // session id 
    std::uint32_t size;              // the size of the payload data 
} net_pkt_header_t;

#pragma pack(pop)

typedef struct {
    net_pkt_header_t header;
    std::string data;               // ip:port
} net_packet_t;

using net_packet_ptr = std::shared_ptr<net_packet_t>;
