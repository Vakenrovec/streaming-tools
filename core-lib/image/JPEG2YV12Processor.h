#pragma once

#include "DataProcessor.h"
#include "MediaPacket.h"

struct AVCodecContext;
struct AVCodec;
struct AVFrame;
struct AVPacket;
struct SwsContext;

class JPEG2YV12Processor: public DataProcessor
{
public:
    JPEG2YV12Processor(int width, int height);

    void Init() override;
    void Destroy() override;

    void Process(const media_packet_ptr& pkt) override;

private:
    bool JPEG2YUV422P(const media_packet_ptr& jpegImage, AVFrame *&yuv422pFrame);
    bool YUV422P2YUV420P(AVFrame *yuv422pFrame, const media_packet_ptr& yuv420pImage);

    bool Decode(AVCodecContext *context, AVFrame *frame, AVPacket *packet);

private:
    int m_width, m_height;
    AVCodec *m_codec;
    AVCodecContext *m_context;
    AVPacket *m_packet;
    AVFrame *m_yuv422pFrame, *m_yuv420pFrame;
    SwsContext *m_imageConvertContext;
};
