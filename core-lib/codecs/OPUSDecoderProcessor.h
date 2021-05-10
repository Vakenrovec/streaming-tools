#pragma once

#include "DataProcessor.h"
#include <memory>

struct opusDecodeContext;
struct AVCodecContext;
struct AVFrame;
struct AVPacket;

class OPUSDecoderProcessor: public DataProcessor
{
public:
    OPUSDecoderProcessor();

    void Init() override;
    void Destroy() override;

    void Process(const media_packet_ptr& pkt) override;

private:
    bool Decode(AVCodecContext *context, AVFrame *frame, AVPacket *packet);
    
private:
    std::shared_ptr<opusDecodeContext> m_decodeContext;
};
