#include "JPEG2YV12Processor.h"
#include "Logger.h"
#include <cstdint>

#ifdef __cplusplus
extern "C" {
#endif
#include <libavcodec/avcodec.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
#ifdef __cplusplus
}
#endif

JPEG2YV12Processor::JPEG2YV12Processor(int width, int height)
: m_codec(nullptr)
, m_context(nullptr)
, m_packet(nullptr)
, m_yuv422pFrame(nullptr)
, m_yuv420pFrame(nullptr)
, m_imageConvertContext(nullptr)
, m_width(width)
, m_height(height)
{
}

void JPEG2YV12Processor::Init()
{
    m_codec = avcodec_find_decoder(AV_CODEC_ID_MJPEG);
    if (!m_codec) {
        LOG_EX_WARN("Codec not found");
        return;
    }
    m_context = avcodec_alloc_context3(m_codec);
    avcodec_open2(m_context, m_codec, nullptr);

    m_packet = av_packet_alloc();
    m_yuv422pFrame = av_frame_alloc();
    m_yuv420pFrame = av_frame_alloc();

    m_imageConvertContext = sws_getContext(
        m_width, m_height, AV_PIX_FMT_YUV422P, 
        m_width, m_height, AV_PIX_FMT_YUV420P,
        SWS_BILINEAR, nullptr, nullptr, nullptr
    ); 

    DataProcessor::Init();
}

void JPEG2YV12Processor::Destroy()
{
    if (m_context) {
        avcodec_free_context(&m_context);
        m_context = nullptr;
    }
    m_codec = nullptr;
    if (m_packet) {
        av_packet_free(&m_packet);
        m_packet = nullptr;
    }
    if (m_yuv422pFrame) {
        av_frame_free(&m_yuv422pFrame);
        m_yuv422pFrame = nullptr;
    }
    if (m_yuv420pFrame) {
        av_frame_free(&m_yuv420pFrame);
        m_yuv420pFrame = nullptr;
    }
    if (m_imageConvertContext) {
        sws_freeContext(m_imageConvertContext);
        m_imageConvertContext = nullptr;
    }    

    DataProcessor::Destroy();
}

void JPEG2YV12Processor::Process(const media_packet_ptr& pkt)
{
    if (pkt->header.type == media_packet_type_t::JPEG)
    {
        JPEG2YUV422P(pkt, m_yuv422pFrame);
        YUV422P2YUV420P(m_yuv422pFrame, pkt);

        DataProcessor::Process(pkt);
    }
    else
    {
        LOG_EX_WARN_WITH_CONTEXT("Incorrect packet type: %d", pkt->header.type);
    }
}

bool JPEG2YV12Processor::JPEG2YUV422P(const media_packet_ptr& jpegImage, AVFrame *&yuv422pFrame)
{
    m_packet->data = jpegImage->data;
    m_packet->size = jpegImage->header.size;
    return Decode(m_context, yuv422pFrame, m_packet);
}

bool JPEG2YV12Processor::YUV422P2YUV420P(AVFrame *yuv422pFrame, const media_packet_ptr& yuv420pImage)
{
    size_t dstDataSize = yuv422pFrame->width * yuv422pFrame->height * 1.5;
    av_image_fill_arrays( 
        m_yuv420pFrame->data,
        m_yuv420pFrame->linesize,
        yuv420pImage->data,
        AV_PIX_FMT_YUV420P,
        yuv422pFrame->width,
        yuv422pFrame->height,
        32
    );

    sws_scale(
        m_imageConvertContext, (uint8_t const * const *)yuv422pFrame->data, yuv422pFrame->linesize, 
        0, yuv422pFrame->height, m_yuv420pFrame->data, m_yuv420pFrame->linesize
    );
    yuv420pImage->header.type = media_packet_type_t::YV12;
    yuv420pImage->header.size = dstDataSize;

    return true;
}

bool JPEG2YV12Processor::Decode(AVCodecContext *context, AVFrame *frame, AVPacket *packet)
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
