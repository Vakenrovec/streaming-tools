#pragma once

#include "DataProcessor.h"
#include <memory>

struct vp8DecodeContext;
struct AVCodecContext;
struct AVFrame;
struct AVPacket;

class VP8DecoderProcessor: public DataProcessor
{
public:
    VP8DecoderProcessor();

    void Init() override;
    void Destroy() override;

    void Process(const media_packet_ptr& pkt) override;

private:
    bool Decode(AVCodecContext *context, AVFrame *frame, AVPacket *packet);
    
private:
    std::shared_ptr<vp8DecodeContext> m_decodeContext;
};
