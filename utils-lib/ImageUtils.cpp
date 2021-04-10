#include "ImageUtils.h"
#include "Logger.h"

#ifdef __cplusplus
extern "C" {
#endif
#include <libavcodec/avcodec.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
#ifdef __cplusplus
}
#endif

bool ImageUtils::JPEG2YUV422P(Image& jpegImage, AVFrame *&yuv422pFrame)
{
    const AVCodec *codec = avcodec_find_decoder(AV_CODEC_ID_MJPEG);
    if (!codec) {
        LOG_EX_WARN("Codec not found");
        return false;
    }
    AVCodecContext *context = avcodec_alloc_context3(codec);
    avcodec_open2(context, codec, NULL);
    AVPacket *packet = av_packet_alloc();
    yuv422pFrame = av_frame_alloc();
    
    packet->data = jpegImage.data;
    packet->size = jpegImage.size;
    Decode(context, yuv422pFrame, packet);

    avcodec_free_context(&context);
    av_packet_free(&packet);

    return true;
}

bool ImageUtils::YUV422P2YUV420P(AVFrame *yuv422pFrame, Image& yuv420pImage)
{
    size_t dstDataSize = yuv422pFrame->width * yuv422pFrame->height * 1.5;
    std::uint8_t* dstData = new std::uint8_t[dstDataSize * sizeof(std::uint8_t)];
    AVFrame *yuv420pFrame = av_frame_alloc();
    av_image_fill_arrays( 
        yuv420pFrame->data,
        yuv420pFrame->linesize,
        dstData,
        AV_PIX_FMT_YUV420P,
        yuv422pFrame->width,
        yuv422pFrame->height,
        32
    );

    struct SwsContext *imageConvertContext = sws_getContext(
        yuv422pFrame->width, yuv422pFrame->height, AV_PIX_FMT_YUV422P, 
        yuv422pFrame->width, yuv422pFrame->height, AV_PIX_FMT_YUV420P,
        SWS_BILINEAR, NULL, NULL, NULL
    );    
    sws_scale(
        imageConvertContext, (uint8_t const * const *)yuv422pFrame->data, yuv422pFrame->linesize, 
        0, yuv422pFrame->height, yuv420pFrame->data, yuv420pFrame->linesize
    );
    yuv420pImage.type = ImageType::YUV420P_RAW;
    yuv420pImage.data = dstData;
    yuv420pImage.size = dstDataSize;

    sws_freeContext(imageConvertContext);
    av_frame_free(&yuv420pFrame);
    av_frame_free(&yuv422pFrame);

    return true;
}

bool ImageUtils::Decode(AVCodecContext *context, AVFrame *frame, AVPacket *packet)
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
