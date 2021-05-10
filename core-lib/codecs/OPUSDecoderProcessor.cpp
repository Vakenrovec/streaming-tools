#include "codecs/OPUSDecoderProcessor.h"
#include "Logger.h"

#ifdef __cplusplus
extern "C" {
#endif
#include <libavcodec/avcodec.h>
#ifdef __cplusplus
}
#endif

typedef struct opusDecodeContext
{
    AVCodecID id = AVCodecID::AV_CODEC_ID_MP2;
    AVCodec *codec = nullptr;
    AVCodecContext *codecContext = nullptr;
    AVPacket *packet = nullptr;
    AVFrame *frame = nullptr;
} opusDecodeContext;

OPUSDecoderProcessor::OPUSDecoderProcessor()
: m_decodeContext(std::make_shared<opusDecodeContext>())
{
}

void OPUSDecoderProcessor::Init()
{
    m_decodeContext->codec = avcodec_find_decoder(m_decodeContext->id);
    if (!m_decodeContext->codec) {
        LOG_EX_WARN("Codec opus not found");
        return;
    }
    m_decodeContext->codecContext = avcodec_alloc_context3(m_decodeContext->codec);
    if (!m_decodeContext->codecContext) {
        LOG_EX_WARN("Could not allocate audio codec context");
        return;
    }
    // m_decodeContext->codecContext->bit_rate = 64000;
    m_decodeContext->codecContext->sample_fmt = AVSampleFormat::AV_SAMPLE_FMT_S16;
    m_decodeContext->codecContext->sample_rate = 44100;
    m_decodeContext->codecContext->channel_layout = AV_CH_LAYOUT_STEREO;
    m_decodeContext->codecContext->channels = av_get_channel_layout_nb_channels(m_decodeContext->codecContext->channel_layout);
    int ret = avcodec_open2(m_decodeContext->codecContext, m_decodeContext->codec, nullptr);
    if (ret < 0) {
        LOG_EX_WARN_WITH_CONTEXT("Could not open audio codec");
        return;
    }

    m_decodeContext->packet = av_packet_alloc();
    if (!m_decodeContext->packet) {
        LOG_EX_WARN("Could not allocate packet");
        return;
    }

    m_decodeContext->frame = av_frame_alloc();
    if (!m_decodeContext->frame) {
        LOG_EX_WARN("Could not allocate audio frame");
        return;
    }
    // m_decodeContext->frame->nb_samples = m_decodeContext->codecContext->frame_size;
    m_decodeContext->frame->format = m_decodeContext->codecContext->sample_fmt;
    m_decodeContext->frame->channel_layout = m_decodeContext->codecContext->channel_layout;

    DataProcessor::Init(); 
}

void OPUSDecoderProcessor::Destroy()
{
    avcodec_free_context(&m_decodeContext->codecContext);
    av_frame_free(&m_decodeContext->frame);
    av_packet_free(&m_decodeContext->packet);

    DataProcessor::Destroy();
}

void OPUSDecoderProcessor::Process(const media_packet_ptr& pkt)
{
    if (pkt->header.type == MediaPacketType::OPUS)
    {
        // m_decodeContext->packet->size = pkt->header.size;
        // m_decodeContext->packet->data = pkt->data;
        
        // if (!Decode(m_decodeContext->codecContext, m_decodeContext->frame, m_decodeContext->packet)) {
        //     LOG_EX_WARN_WITH_CONTEXT("Frame wasn't decoded");
        //     return;
        // }
        // LOG_EX_INFO_WITH_CONTEXT("Frame was decoded");

        pkt->header.type = MediaPacketType::AUDIO_SAMPLES;
        // pkt->header.size = m_decodeContext->frame->nb_samples * 
        //     av_get_bytes_per_sample(m_decodeContext->codecContext->sample_fmt) * 
        //     m_decodeContext->codecContext->channels;
        // std::copy(
        //     m_decodeContext->frame->data[0], 
        //     m_decodeContext->frame->data[0] + m_decodeContext->frame->nb_samples * 
        //         av_get_bytes_per_sample(m_decodeContext->codecContext->sample_fmt), 
        //     pkt->data
        // );
        // std::copy(
        //     m_decodeContext->frame->data[1], 
        //     m_decodeContext->frame->data[1] + m_decodeContext->frame->nb_samples * 
        //         av_get_bytes_per_sample(m_decodeContext->codecContext->sample_fmt), 
        //     pkt->data + m_decodeContext->frame->nb_samples * 
        //         av_get_bytes_per_sample(m_decodeContext->codecContext->sample_fmt)
        // );

        DataProcessor::Process(pkt);
    }
    else
    {
        LOG_EX_WARN_WITH_CONTEXT("Incorrect packet type: %d", pkt->header.type);
    }
}

bool OPUSDecoderProcessor::Decode(AVCodecContext *context, AVFrame *frame, AVPacket *packet)
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
