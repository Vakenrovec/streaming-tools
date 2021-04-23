#include "program.h"

#include "devices/WebCamera.h"
#include "codecs/vp8.h"
#include "Image.h"
#include "ImageUtils.h"
#include "Logger.h"
#include "rtp/RTPFragmenter.h"
#include "StreamerSession.h"
#include "ThreadPoolManager.h"
#include "MediaPacket.h"
#include <fstream>
#include <cstdint>
#include <memory>
#include <list>

#ifdef __cplusplus
extern "C" {
#endif
#include <libavcodec/avcodec.h>
#ifdef __cplusplus
}
#endif

int main() {
    int width = 1280, height = 720, gopSize = 5, bitrate = 400000;
    
    VP8Codec vp8codec;
    vp8codec.InitEncodeContext(width, height, gopSize, bitrate);
    vp8codec.InitDecodeContext();

    std::uint16_t threadsCount = 1;
    ThreadPoolManager::GetInstance()->Start(threadsCount);
    auto session = std::make_shared<StreamerSession>(ThreadPoolManager::GetInstance()->Get());
    session->SetFPS(30);
    session->SetServerTcpEndpoint("192.11.0.3", 35005);
    session->SetServerUdpEndpoint("192.11.0.3", 35006);
    session->SetLocalUdpEndpoint("192.11.0.3", 35007);
    session->SetLocalUdpIp("192.11.0.3");
    session->SetLocalUdpPort(35007);
    std::uint32_t id = 777;
    session->CreateStream(id);

    WebCamera webCamera;
    webCamera.Initialize(width, height);
    for (int i = 0; i < 1000; i++)
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

        RTPFragmenter fragmenter;
        auto packets = fragmenter.FragmentRTPFrame(vp8Image);       
        // transfer over the network        
        for (const auto& pkt : packets)
        {
            session->WriteData(pkt);
        }                
        // ... 
        auto img = fragmenter.DefragmentRTPPackets(packets);

        Image yv12Image;
        vp8codec.Decode(img, yv12Image);
        outFile.open("webcam_output" + std::to_string(i) + ".yv12", std::ios::binary | std::ios::trunc);
        outFile.write((char*)yv12Image.data, yv12Image.size);
        outFile.close();
        delete[] yv12Image.data;

        // std::this_thread::sleep_for(std::chrono::seconds(2));
    }
    webCamera.Deinitialize();
    vp8codec.DeinitEncodeContext();
    vp8codec.DeinitDecodeContext();
    return 0;
}
