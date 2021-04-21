#pragma once

#include <memory>

struct Image;
struct vp8encodeContext;
struct vp8decodeContext;
struct AVCodecContext;
struct AVFrame;
struct AVPacket;

class VP8Codec
{
public:
    VP8Codec();

    bool InitEncodeContext(int width, int height, int gopSize, int bitrate);
    void DeinitEncodeContext();
    bool InitDecodeContext();
    void DeinitDecodeContext();
    bool Encode(Image& yv12image, Image& vp8image);
    bool Decode(Image& vp8image, Image& yv12image);

private:
    bool encode(AVCodecContext *context, AVFrame *frame, AVPacket *packet);
    bool decode(AVCodecContext *context, AVFrame *frame, AVPacket *packet);
    
private:
    std::shared_ptr<vp8encodeContext> m_encodeContext;
    std::shared_ptr<vp8decodeContext> m_decodeContext;
};
