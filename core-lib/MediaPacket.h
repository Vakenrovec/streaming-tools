#pragma once

#include <cstdint>
#include <vector>
#include <memory>

typedef enum MediaPacketType: std::uint8_t {
    UNKNOWN = 0,
    AUDIO_SAMPLES = 1,
    OPUS = 2,
    AUDIO_RTP = 3,
    JPEG = 4,
    YUV422P = 5,
    YV12 = 6,
    VP8 = 7,
    VIDEO_RTP = 8,
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
