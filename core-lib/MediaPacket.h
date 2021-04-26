#pragma once

#include <cstdint>
#include <vector>
#include <memory>

typedef enum MediaPacketType: std::uint8_t {
    UNKNOWN = 0,
    AUDIO = 1,
    JPEG = 2,
    YUV422P = 3,
    YV12 = 4,
    VP8 = 5,
    RTP = 6,
} media_packet_type_t;

#pragma pack(push, 1)

typedef struct {
    MediaPacketType type;        /* payload type */
    std::uint64_t ts;            /* timestamp */
    std::uint32_t size;          /* the size of the payload data */
} media_pkt_header_t;

#pragma pack(pop)

constexpr inline std::uint32_t MediaPacketDataSize = 3 * 1024 * 1024;

struct media_packet_t {
    media_pkt_header_t header;
    std::uint8_t data[MediaPacketDataSize];
};

using media_packet_ptr = std::shared_ptr<media_packet_t>;
