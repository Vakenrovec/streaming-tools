#pragma once

#include "DataProcessor.h"
#include <memory>

struct Image;
struct vp8encodeContext;
struct AVCodecContext;
struct AVFrame;
struct AVPacket;

class VP8EncoderProcessor: public DataProcessor
{
public:
    VP8EncoderProcessor(int width, int height, int gopSize, int bitrate);

    void Init() override;
    void Destroy() override;

    void Process(const media_packet_ptr& pkt) override;

private:
    bool Encode(AVCodecContext *context, AVFrame *frame, AVPacket *packet);
    
private:
    std::shared_ptr<vp8encodeContext> m_encodeContext;
    int m_width, m_height, m_gopSize, m_bitrate;
};
