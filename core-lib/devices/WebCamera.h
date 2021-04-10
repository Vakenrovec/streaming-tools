#pragma once

#include "ImageUtils.h"
#include <linux/videodev2.h>
#include <cstdint>

class WebCamera
{
public:
    WebCamera();
    bool Initialize(int width, int height, unsigned int pixelformat = V4L2_PIX_FMT_JPEG);
    bool Deinitialize();
    bool GetFrame(Image& image);

private:
    bool Open();
    bool IsCanCapture();
    bool SetImageFormat(int width, int height, unsigned int pixelformat);
    bool RequestBuffer();
    bool QueryBuffer();
    bool Start();
    bool Stop();

private:
    int m_descriptor;
    std::uint8_t* m_buffer;
    v4l2_buffer m_bufferInfo;
};
