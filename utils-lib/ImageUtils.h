#pragma once

#include <cstdint>
#include "Image.h"

struct AVCodecContext;
struct AVFrame;
struct AVPacket;

class ImageUtils
{
public:
    ImageUtils() = delete;

    static bool JPEG2YUV422P(Image& jpegImage, AVFrame *&yuv422pFrame);
    static bool YUV422P2YUV420P(AVFrame *yuv422pFrame, Image& yuv420pImage);

private:
    static bool Decode(AVCodecContext *context, AVFrame *frame, AVPacket *packet);
};
