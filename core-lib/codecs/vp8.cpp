#include "vp8.h"
#include "Logger.h"
#include "ImageUtils.h"

#ifdef __cplusplus
extern "C" {
#endif
#include <libavcodec/avcodec.h>
#ifdef __cplusplus
}
#endif

typedef struct vp8encodeContext
{
    AVCodecID id = AVCodecID::AV_CODEC_ID_VP8;
    AVCodec *codec = nullptr;
    AVCodecContext *codecContext = nullptr;
    AVPacket *packet = nullptr;
    AVFrame *frame = nullptr;
} vp8encodeContext;

typedef struct vp8decodeContext
{
    AVCodecID id = AVCodecID::AV_CODEC_ID_VP8;
    AVCodec *codec = nullptr;
    AVCodecContext *codecContext = nullptr;
    AVPacket *packet = nullptr;
    AVFrame *frame = nullptr;
} vp8decodeContext;

VP8Codec::VP8Codec()
: m_encodeContext(std::make_shared<vp8encodeContext>())
, m_decodeContext(std::make_shared<vp8decodeContext>())
{
}

bool VP8Codec::InitEncodeContext(int width, int height, int gopSize, int bitrate)
{
    m_encodeContext->codec = avcodec_find_encoder(m_encodeContext->id);
    if (!m_encodeContext->codec) {
        LOG_EX_WARN("Codec vp8 not found");
        return false;
    }
    m_encodeContext->codecContext = avcodec_alloc_context3(m_encodeContext->codec);
    if (!m_encodeContext->codecContext) {
        LOG_EX_WARN("Could not allocate video codec context");
        return false;
    }
    m_encodeContext->codecContext->width = width;
    m_encodeContext->codecContext->height = height;
    m_encodeContext->codecContext->gop_size = gopSize;
    m_encodeContext->codecContext->pix_fmt = AV_PIX_FMT_YUV420P;
    m_encodeContext->codecContext->time_base = AVRational{1, 25};
    m_encodeContext->codecContext->bit_rate = bitrate;
    int ret = avcodec_open2(m_encodeContext->codecContext, m_encodeContext->codec, nullptr);
    if (ret < 0) {
        LOG_EX_WARN("Could not open codec");
        return false;
    }

    m_encodeContext->packet = av_packet_alloc();
    if (!m_encodeContext->packet) {
        LOG_EX_WARN("Could not allocate packet");
        return false;
    }

    m_encodeContext->frame = av_frame_alloc();
    if (!m_encodeContext->frame) {
        LOG_EX_WARN("Could not allocate video frame");
        return false;
    }
    m_encodeContext->frame->format = m_encodeContext->codecContext->pix_fmt;
    m_encodeContext->frame->width  = m_encodeContext->codecContext->width;
    m_encodeContext->frame->height = m_encodeContext->codecContext->height;
    ret = av_frame_get_buffer(m_encodeContext->frame, 0);
    if (ret < 0) {
        LOG_EX_WARN("Could not allocate the video frame data");
        return false;
    }
    ret = av_frame_make_writable(m_encodeContext->frame);
    if (ret < 0) {
        LOG_EX_WARN("Could not set frame as writable");
        return false;
    }
    return true;
}

void VP8Codec::DeinitEncodeContext()
{
    avcodec_free_context(&m_encodeContext->codecContext);
    av_frame_free(&m_encodeContext->frame);
    av_packet_free(&m_encodeContext->packet);
}

bool VP8Codec::InitDecodeContext()
{
    m_decodeContext->codec = avcodec_find_decoder(m_decodeContext->id);
    if (!m_decodeContext->codec) {
        LOG_EX_WARN("Codec vp8 not found");
        return false;
    }
    m_decodeContext->codecContext = avcodec_alloc_context3(m_decodeContext->codec);
    if (!m_decodeContext->codecContext) {
        LOG_EX_WARN("Could not allocate video codec context");
        return false;
    }
    int ret = avcodec_open2(m_decodeContext->codecContext, m_decodeContext->codec, nullptr);
    if (ret < 0) {
        LOG_EX_WARN("Could not open codec");
        return false;
    }

    m_decodeContext->packet = av_packet_alloc();
    if (!m_decodeContext->packet) {
        LOG_EX_WARN("Could not allocate packet");
        return false;
    }

    m_decodeContext->frame = av_frame_alloc();
    if (!m_decodeContext->frame) {
        LOG_EX_WARN("Could not allocate video frame");
        return false;
    }
    // ret = av_frame_get_buffer(m_decodeContext->frame, 0);
    // if (ret < 0) {
    //     LOG_EX_WARN("Could not allocate the video frame data");
    //     return false;
    // }
    // ret = av_frame_make_writable(m_decodeContext->frame);
    // if (ret < 0) {
    //     LOG_EX_WARN("Could not set frame as writable");
    //     return false;
    // }
    return true;
}

void VP8Codec::DeinitDecodeContext()
{
    avcodec_free_context(&m_decodeContext->codecContext);
    av_frame_free(&m_decodeContext->frame);
    av_packet_free(&m_decodeContext->packet);
}

bool VP8Codec::Encode(Image& yv12image, Image& vp8image)
{
    m_encodeContext->frame->data[0] = yv12image.data;
    m_encodeContext->frame->data[1] = yv12image.data + m_encodeContext->frame->width * m_encodeContext->frame->height;
    m_encodeContext->frame->data[2] = yv12image.data + m_encodeContext->frame->width * m_encodeContext->frame->height * 5 / 4;

    if (!encode(m_encodeContext->codecContext, m_encodeContext->frame, m_encodeContext->packet)) {
        return false;
    }
    LOG_EX_INFO("Frame was encoded");
    vp8image.type = ImageType::VP8;
    vp8image.data = m_encodeContext->packet->data;
    vp8image.size = m_encodeContext->packet->size;

    return true;
}

bool VP8Codec::encode(AVCodecContext *context, AVFrame *frame, AVPacket *packet)
{
    int ret = avcodec_send_frame(context, frame);
    if (ret < 0) {
        LOG_EX_WARN("Error sending a frame for encoding");
        return false;
    }

    ret = avcodec_receive_packet(context, packet);
    if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
        return false;
    else if (ret < 0) {
        LOG_EX_WARN("Error during encoding");
        return false;
    }

    return true;
}

bool VP8Codec::Decode(Image& vp8image, Image& yv12image)
{
    m_decodeContext->packet->data = vp8image.data;
    m_decodeContext->packet->size = vp8image.size;
    decode(m_decodeContext->codecContext, m_decodeContext->frame, m_decodeContext->packet);
    LOG_EX_INFO("Frame was decoded, key = " + std::to_string(m_decodeContext->frame->key_frame));

    yv12image.type = ImageType::YUV420P_RAW;
    yv12image.size = m_decodeContext->frame->width * m_decodeContext->frame->height * 3 / 2;
    yv12image.data = new std::uint8_t[yv12image.size * sizeof(std::uint8_t)];
    std::copy(
        m_decodeContext->frame->data[0], 
        m_decodeContext->frame->data[0] + m_decodeContext->frame->linesize[0] * m_decodeContext->frame->height, 
        yv12image.data
    );
    std::copy(
        m_decodeContext->frame->data[1], 
        m_decodeContext->frame->data[1] + m_decodeContext->frame->linesize[1] * m_decodeContext->frame->height / 2, 
        yv12image.data + m_decodeContext->frame->linesize[0] * m_decodeContext->frame->height
    );
    std::copy(
        m_decodeContext->frame->data[2], 
        m_decodeContext->frame->data[2] + m_decodeContext->frame->linesize[2] * m_decodeContext->frame->height / 2, 
        yv12image.data + m_decodeContext->frame->linesize[0] * m_decodeContext->frame->height + m_decodeContext->frame->linesize[1] * m_decodeContext->frame->height / 2
    );

    return true;
}

bool VP8Codec::decode(AVCodecContext *context, AVFrame *frame, AVPacket *packet)
{
    int ret = avcodec_send_packet(context, packet);
    if (ret < 0) {
        LOG_EX_WARN("Error sending a packet for decoding");
        return false;
    }
    ret = avcodec_receive_frame(context, frame);
    if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF){
        LOG_EX_WARN("Error receiving a packet from decoding");
        return false;
    }
    else if (ret < 0) {
        LOG_EX_WARN("Error receiving a packet from decoding");
        return false;
    }
    return true;
}
