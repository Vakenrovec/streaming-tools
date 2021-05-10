#include "VP8EncoderProcessor.h"
#include "Logger.h"

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

VP8EncoderProcessor::VP8EncoderProcessor(int width, int height, int gopSize, int bitrate)
: m_encodeContext(std::make_shared<vp8encodeContext>())
, m_width(width)
, m_height(height)
, m_gopSize(gopSize)
, m_bitrate(bitrate)
{
}

void VP8EncoderProcessor::Init()
{
    m_encodeContext->codec = avcodec_find_encoder(m_encodeContext->id);
    if (!m_encodeContext->codec) {
        LOG_EX_WARN("Codec vp8 not found");
        return;
    }
    m_encodeContext->codecContext = avcodec_alloc_context3(m_encodeContext->codec);
    if (!m_encodeContext->codecContext) {
        LOG_EX_WARN("Could not allocate video codec context");
        return;
    }
    m_encodeContext->codecContext->width = m_width;
    m_encodeContext->codecContext->height = m_height;
    m_encodeContext->codecContext->gop_size = m_gopSize;
    m_encodeContext->codecContext->pix_fmt = AV_PIX_FMT_YUV420P;
    m_encodeContext->codecContext->time_base = AVRational{1, 25};
    m_encodeContext->codecContext->bit_rate = m_bitrate;
    int ret = avcodec_open2(m_encodeContext->codecContext, m_encodeContext->codec, nullptr);
    if (ret < 0) {
        LOG_EX_WARN("Could not open vp8 codec");
        return;
    }

    m_encodeContext->packet = av_packet_alloc();
    if (!m_encodeContext->packet) {
        LOG_EX_WARN("Could not allocate packet");
        return;
    }

    m_encodeContext->frame = av_frame_alloc();
    if (!m_encodeContext->frame) {
        LOG_EX_WARN("Could not allocate video frame");
        return;
    }
    m_encodeContext->frame->format = m_encodeContext->codecContext->pix_fmt;
    m_encodeContext->frame->width  = m_encodeContext->codecContext->width;
    m_encodeContext->frame->height = m_encodeContext->codecContext->height;
    ret = av_frame_get_buffer(m_encodeContext->frame, 0);
    if (ret < 0) {
        LOG_EX_WARN("Could not allocate the video frame data");
        return;
    }
    ret = av_frame_make_writable(m_encodeContext->frame);
    if (ret < 0) {
        LOG_EX_WARN("Could not set frame as writable");
        return;
    }
    
    DataProcessor::Init();
}

void VP8EncoderProcessor::Destroy()
{
    avcodec_free_context(&m_encodeContext->codecContext);
    av_frame_free(&m_encodeContext->frame);
    av_packet_free(&m_encodeContext->packet);

    DataProcessor::Destroy();
}

void VP8EncoderProcessor::Process(const media_packet_ptr& pkt)
{
    if (pkt->header.type == MediaPacketType::YV12)
    {
        m_encodeContext->frame->data[0] = pkt->data;
        m_encodeContext->frame->data[1] = pkt->data + m_encodeContext->frame->width * m_encodeContext->frame->height;
        m_encodeContext->frame->data[2] = pkt->data + m_encodeContext->frame->width * m_encodeContext->frame->height * 5 / 4;

        if (!Encode(m_encodeContext->codecContext, m_encodeContext->frame, m_encodeContext->packet)) {
            LOG_EX_WARN_WITH_CONTEXT("Frame wasn't encoded");
            return;
        }
        // LOG_EX_INFO_WITH_CONTEXT("Frame was encoded");
        pkt->header.type = MediaPacketType::VP8;
        pkt->header.size = m_encodeContext->packet->size;
        std::copy(m_encodeContext->packet->data, m_encodeContext->packet->data + m_encodeContext->packet->size, pkt->data);    
    
        DataProcessor::Process(pkt);
    }
    else
    {
        LOG_EX_WARN_WITH_CONTEXT("Incorrect packet type: %d", pkt->header.type);
    }
}

bool VP8EncoderProcessor::Encode(AVCodecContext *context, AVFrame *frame, AVPacket *packet)
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
