#pragma once

#include "DataProcessor.h"
#include <memory>

struct opusEncodeContext;
struct AVCodecContext;
struct AVFrame;
struct AVPacket;

class OPUSEncoderProcessor: public DataProcessor
{
public:
    OPUSEncoderProcessor(int bitrate = 64000);

    void Init() override;
    void Destroy() override;

    void Process(const media_packet_ptr& pkt) override;

private:
    bool Encode(AVCodecContext *context, AVFrame *frame, AVPacket *packet);

private:
    std::shared_ptr<opusEncodeContext> m_encodeContext;
    int m_bitrate;
};
