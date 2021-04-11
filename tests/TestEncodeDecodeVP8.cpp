#include "catch.hpp"
#include "codecs/vp8.h"
#include "Image.h"
#include "ImageUtils.h"
#include "Logger.h"
#include "rtp/RTPFragmenter.h"
#include "FileUtils.h"
#include <fstream>
#include <string>
#include <cstdint>
#include <vector>

#ifdef __cplusplus
extern "C" {
#endif
#include <libavcodec/avcodec.h>
#ifdef __cplusplus
}
#endif

TEST_CASE("Encode-decode-yv12", "[encode][decode]") {
    int width = 1280, height = 720, gopSize = 10;
    VP8Codec vp8codec;
    vp8codec.InitEncodeContext(width, height, gopSize);
    vp8codec.InitDecodeContext();

    std::vector<std::uint8_t> imageData;
    FileUtils::ReadFile("/media/sf_janus-tools/tests/test-data/webcam_output.yuv420p", imageData);
    Image yuv420pImage;
    yuv420pImage.data = imageData.data();
    Image vp8Image;
    vp8codec.Encode(yuv420pImage, vp8Image);

    yuv420pImage.data = nullptr;
    vp8codec.Decode(vp8Image, yuv420pImage);
    std::ofstream outFile;
    outFile.open("webcam_output.yv12", std::ios::binary | std::ios::trunc);
    outFile.write((char*)yuv420pImage.data, yuv420pImage.size);
    outFile.close();
    delete[] yuv420pImage.data;
    vp8codec.DeinitEncodeContext();
    vp8codec.DeinitDecodeContext();
}

TEST_CASE("Encode-fragment-defragment-decode-yv12", "[encode][decode][fragment][defragment]") {
    int width = 1280, height = 720, gopSize = 10;
    VP8Codec vp8codec;
    vp8codec.InitEncodeContext(width, height, gopSize);
    vp8codec.InitDecodeContext();

    std::vector<std::uint8_t> imageData;
    FileUtils::ReadFile("/media/sf_janus-tools/tests/test-data/webcam_output.yuv420p", imageData);
    Image yuv420pImage;
    yuv420pImage.data = imageData.data();
    Image vp8Image;
    vp8codec.Encode(yuv420pImage, vp8Image);

    RTPFragmenter fragmenter;
    auto packets = fragmenter.FragmentRTPFrame(vp8Image);
    auto img = fragmenter.DefragmentRTPPackets(packets);

    yuv420pImage.data = nullptr;
    vp8codec.Decode(img, yuv420pImage);  // vp8Image instead of img
    FileUtils::WriteFile("webcam_output.yv12", (char*)yuv420pImage.data, yuv420pImage.size);
    delete[] yuv420pImage.data;
    vp8codec.DeinitEncodeContext();
    vp8codec.DeinitDecodeContext();
}
