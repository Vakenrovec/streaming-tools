#pragma once

#include <cstdint>

typedef enum class ImageType {
    Unknown = 0,
    JPEG = 1,
    YUV420P_RAW = 2,
    YUV422P_RAW = 3,
    VP8 = 4,
} ImageType;

typedef struct Image {
    ImageType type;
    std::uint32_t size;
    std::uint8_t* data;
    int width;
    int height;
} Image;
