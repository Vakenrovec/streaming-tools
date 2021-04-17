#pragma once

#include <cstdint>
#include <vector>
#include <memory>

typedef enum PacketType: std::uint8_t {
    UNKNOWN = 0,
    AUDIO = 1,
    VIDEO_RTP = 2,
} packet_type_t;

#pragma pack(push, 1)

typedef struct {
   PacketType type;         /* payload type */
   std::uint64_t ts;        /* timestamp */
   std::uint32_t size;      /* the size of the payload data */
} media_pkt_header_t;

#pragma pack(pop)

struct media_packet_t {
    media_pkt_header_t header;
    std::uint8_t *data;
    ~media_packet_t(){ delete[] data; };
};

#define MEDIA_PACKET_SIZE_BYTES sizeof(media_pkt_header_t)

using media_packet_ptr = std::shared_ptr<media_packet_t>;
