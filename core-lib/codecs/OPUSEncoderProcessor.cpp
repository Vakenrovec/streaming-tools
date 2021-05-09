#include "codecs/OPUSEncoderProcessor.h"
#include "Logger.h"

#ifdef __cplusplus
extern "C" {
#endif
#include <libavcodec/avcodec.h>
#ifdef __cplusplus
}
#endif

typedef struct opusEncodeContext
{
    AVCodecID id = AVCodecID::AV_CODEC_ID_VORBIS; // AV_CODEC_ID_VORBIS AV_CODEC_ID_OPUS
    AVCodec *codec = nullptr;
    AVCodecContext *codecContext = nullptr;
    AVPacket *packet = nullptr;
    AVFrame *frame = nullptr;
} opusEncodeContext;

OPUSEncoderProcessor::OPUSEncoderProcessor(int bitrate)
: m_encodeContext(std::make_shared<opusEncodeContext>())
, m_bitrate(bitrate)
{
}

void OPUSEncoderProcessor::Init()
{
    m_encodeContext->codec = avcodec_find_encoder(m_encodeContext->id);
    if (!m_encodeContext->codec) {
        LOG_EX_WARN("Codec opus not found");
        return;
    }
    m_encodeContext->codecContext = avcodec_alloc_context3(m_encodeContext->codec);
    if (!m_encodeContext->codecContext) {
        LOG_EX_WARN("Could not allocate audio codec context");
        return;
    }
    m_encodeContext->codecContext->bit_rate = 400000; //m_bitrate;
    m_encodeContext->codecContext->sample_fmt = AVSampleFormat::AV_SAMPLE_FMT_FLTP;
    m_encodeContext->codecContext->sample_rate = 44100;
    m_encodeContext->codecContext->channel_layout = AV_CH_LAYOUT_STEREO;
    m_encodeContext->codecContext->channels = av_get_channel_layout_nb_channels(m_encodeContext->codecContext->channel_layout);
    int ret = avcodec_open2(m_encodeContext->codecContext, m_encodeContext->codec, nullptr);
    if (ret < 0) {
        LOG_EX_WARN("Could not open opus codec");
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
    m_encodeContext->frame->nb_samples = m_encodeContext->codecContext->frame_size;
    m_encodeContext->frame->format  = m_encodeContext->codecContext->sample_fmt;
    m_encodeContext->frame->channel_layout = m_encodeContext->codecContext->channel_layout;
    ret = av_frame_get_buffer(m_encodeContext->frame, 0);
    if (ret < 0) {
        LOG_EX_WARN("Could not allocate the audio frame data");
        return;
    }
    ret = av_frame_make_writable(m_encodeContext->frame);
    if (ret < 0) {
        LOG_EX_WARN("Could not set frame as writable");
        return;
    }

    DataProcessor::Init();    
}

void OPUSEncoderProcessor::Destroy()
{
    avcodec_free_context(&m_encodeContext->codecContext);
    av_frame_free(&m_encodeContext->frame);
    av_packet_free(&m_encodeContext->packet);

    DataProcessor::Destroy();
}

void OPUSEncoderProcessor::Process(const media_packet_ptr& pkt)
{
    if (pkt->header.type == MediaPacketType::AUDIO_SAMPLES)
    {
        m_encodeContext->frame->data[0] = pkt->data;
        
        if (!Encode(m_encodeContext->codecContext, m_encodeContext->frame, m_encodeContext->packet)) {
            LOG_EX_WARN("Frame wasn't encoded");
            return;
        }
        LOG_EX_INFO("Frame was encoded");
        pkt->header.type = MediaPacketType::OPUS;
        pkt->header.size = m_encodeContext->packet->size;
        std::copy(m_encodeContext->packet->data, m_encodeContext->packet->data + m_encodeContext->packet->size, pkt->data);    

        DataProcessor::Process(pkt);
    }
    else
    {
        LOG_EX_WARN_WITH_CONTEXT("Incorrect packet type: %d", pkt->header.type);
    }
}

bool OPUSEncoderProcessor::Encode(AVCodecContext *context, AVFrame *frame, AVPacket *packet)
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
