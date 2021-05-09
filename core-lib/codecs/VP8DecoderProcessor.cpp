#include "VP8DecoderProcessor.h"
#include "Logger.h"

#ifdef __cplusplus
extern "C" {
#endif
#include <libavcodec/avcodec.h>
#ifdef __cplusplus
}
#endif

typedef struct vp8decodeContext
{
    AVCodecID id = AVCodecID::AV_CODEC_ID_VP8;
    AVCodec *codec = nullptr;
    AVCodecContext *codecContext = nullptr;
    AVPacket *packet = nullptr;
    AVFrame *frame = nullptr;
} vp8decodeContext;

VP8DecoderProcessor::VP8DecoderProcessor()
: m_decodeContext(std::make_shared<vp8decodeContext>())
{
}


void VP8DecoderProcessor::Init()
{
    m_decodeContext->codec = avcodec_find_decoder(m_decodeContext->id);
    if (!m_decodeContext->codec) {
        LOG_EX_WARN("Codec vp8 not found");
        return;
    }
    m_decodeContext->codecContext = avcodec_alloc_context3(m_decodeContext->codec);
    if (!m_decodeContext->codecContext) {
        LOG_EX_WARN("Could not allocate video codec context");
        return;
    }
    int ret = avcodec_open2(m_decodeContext->codecContext, m_decodeContext->codec, nullptr);
    if (ret < 0) {
        LOG_EX_WARN("Could not open codec");
        return;
    }

    m_decodeContext->packet = av_packet_alloc();
    if (!m_decodeContext->packet) {
        LOG_EX_WARN("Could not allocate packet");
        return;
    }

    m_decodeContext->frame = av_frame_alloc();
    if (!m_decodeContext->frame) {
        LOG_EX_WARN("Could not allocate video frame");
        return;
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

    DataProcessor::Init();
}

void VP8DecoderProcessor::Destroy()
{
    avcodec_free_context(&m_decodeContext->codecContext);
    av_frame_free(&m_decodeContext->frame);
    av_packet_free(&m_decodeContext->packet);

    DataProcessor::Destroy();
}

void VP8DecoderProcessor::Process(const media_packet_ptr& pkt)
{
    if (pkt->header.type == media_packet_type_t::VP8)
    {
        m_decodeContext->packet->size = pkt->header.size;
        m_decodeContext->packet->data = pkt->data;
        
        if (!Decode(m_decodeContext->codecContext, m_decodeContext->frame, m_decodeContext->packet)) {
            LOG_EX_WARN("Frame wasn't decoded, key = " + std::to_string(m_decodeContext->frame->key_frame));
            return;
        }
        LOG_EX_INFO("Frame was decoded, key = " + std::to_string(m_decodeContext->frame->key_frame));

        pkt->header.type = MediaPacketType::YV12;
        pkt->header.size = m_decodeContext->frame->width * m_decodeContext->frame->height * 3 / 2;
        std::copy(
            m_decodeContext->frame->data[0], 
            m_decodeContext->frame->data[0] + m_decodeContext->frame->linesize[0] * m_decodeContext->frame->height, 
            pkt->data
        );
        std::copy(
            m_decodeContext->frame->data[1], 
            m_decodeContext->frame->data[1] + m_decodeContext->frame->linesize[1] * m_decodeContext->frame->height / 2, 
            pkt->data + m_decodeContext->frame->linesize[0] * m_decodeContext->frame->height
        );
        std::copy(
            m_decodeContext->frame->data[2], 
            m_decodeContext->frame->data[2] + m_decodeContext->frame->linesize[2] * m_decodeContext->frame->height / 2, 
            pkt->data + m_decodeContext->frame->linesize[0] * m_decodeContext->frame->height + m_decodeContext->frame->linesize[1] * m_decodeContext->frame->height / 2
        );

        DataProcessor::Process(pkt);
    }
    else
    {
        LOG_EX_WARN_WITH_CONTEXT("Incorrect packet type: %d", pkt->header.type);
    }
}

bool VP8DecoderProcessor::Decode(AVCodecContext *context, AVFrame *frame, AVPacket *packet)
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
