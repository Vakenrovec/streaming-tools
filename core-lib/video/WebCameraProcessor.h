#pragma once

#include "PlayableDataProcessor.h"
#include "MediaPacket.h"
#include <linux/videodev2.h>
#include <cstdint>

class WebCameraProcessor: public PlayableDataProcessor
{
public:
    WebCameraProcessor(int width, int height, std::uint32_t pixelformat = V4L2_PIX_FMT_JPEG);

    void Init() override;
    void Destroy() override;

    void Process(const media_packet_ptr& pkt) override;
    
private:
    bool Open();
    bool IsCanCapture();
    bool SetImageFormat(int width, int height, std::uint32_t pixelformat);
    bool RequestBuffer();
    bool QueryBuffer();
    bool Start();
    bool Close();

    bool GetFrame(const media_packet_ptr& pkt);

private:
    int m_width, m_height;
    std::uint32_t m_pixelformat;

    int m_descriptor;
    std::uint8_t* m_buffer;
    v4l2_buffer m_bufferInfo;
};
