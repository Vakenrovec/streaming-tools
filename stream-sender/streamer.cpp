#include "streamer.h"

#include "devices/WebCamera.h"
#include "codecs/vp8.h"
#include "Image.h"
#include "ImageUtils.h"
#include "Logger.h"

#ifdef __cplusplus
extern "C" {
#endif
#include <libavcodec/avcodec.h>
#ifdef __cplusplus
}
#endif

#include <fstream>

int main() {
    int width = 1280, height = 720;
    
    VP8Codec vp8codec;
    int gopSize = 10;
    vp8codec.InitEncodeContext(width, height, gopSize);
    vp8codec.InitDecodeContext();

    WebCamera webCamera;
    webCamera.Initialize(width, height);
    for (int i = 0; i < 2000; i++)
    {
        Image jpegImage;
        
        webCamera.GetFrame(jpegImage);
        std::ofstream outFile;
        outFile.open("webcam_output" + std::to_string(i) + ".jpeg", std::ios::binary | std::ios::trunc);
        outFile.write((char*)jpegImage.data, jpegImage.size);
        outFile.close();

        AVFrame *yuv422pFrame = nullptr;
        ImageUtils::JPEG2YUV422P(jpegImage, yuv422pFrame);
        delete[] jpegImage.data;
        Image yuv420pImage;
        ImageUtils::YUV422P2YUV420P(yuv422pFrame, yuv420pImage);
        
        outFile.open("webcam_output" + std::to_string(i) + ".yuv420p", std::ios::binary | std::ios::trunc);
        outFile.write((char*)yuv420pImage.data, yuv420pImage.size);
        outFile.close();
        
        Image vp8Image;
        vp8codec.Encode(yuv420pImage, vp8Image);
        outFile.open("webcam_output" + std::to_string(i) + ".vp8", std::ios::binary | std::ios::trunc);
        outFile.write((char*)vp8Image.data, vp8Image.size);
        outFile.close();
        delete[] yuv420pImage.data; 

        Image yv12Image;
        vp8codec.Decode(vp8Image, yv12Image);
        outFile.open("webcam_output" + std::to_string(i) + ".yv12", std::ios::binary | std::ios::trunc);
        outFile.write((char*)yv12Image.data, yv12Image.size);
        outFile.close();
        delete[] yv12Image.data;
    }
    webCamera.Deinitialize();
    vp8codec.DeinitEncodeContext();
    vp8codec.DeinitDecodeContext();
    return 0;
}
