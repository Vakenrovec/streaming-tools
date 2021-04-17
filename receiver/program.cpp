#include "program.h"

#include "devices/WebCamera.h"
#include "codecs/vp8.h"
#include "Image.h"
#include "ImageUtils.h"
#include "Logger.h"
#include "rtp/RTPFragmenter.h"
#include "ReceiverSession.h"
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
    VP8Codec vp8codec;
    vp8codec.InitDecodeContext();

    std::uint16_t threadsCount = 1;
    ThreadPoolManager::GetInstance()->Start(threadsCount);
    auto session = std::make_shared<ReceiverSession>(ThreadPoolManager::GetInstance()->Get());
    session->SetFPS(30);
    session->SetServerTcpEndpoint("127.0.0.1", 35005);
    session->SetServerUdpEndpoint("127.0.0.1", 35006);
    session->SetLocalUdpEndpoint("127.0.0.1", 35008);
    session->SetLocalUdpIp("127.0.0.1");
    session->SetLocalUdpPort(35008);
    std::uint32_t id = 777;
    session->ConnectToStream(id);

    RTPFragmenter fragmenter; 

    for (int i = 0; i < 1; i++)
    {
        std::list<media_packet_ptr> packets;        
        // get pkts over the network
        // for (const auto& pkt : packets)
        // {
        //     session->ReadData(pkt);
        // }        
        // // ... 
        std::this_thread::sleep_for(std::chrono::seconds(100));
        auto img = fragmenter.DefragmentRTPPackets(packets);

        Image yv12Image;
        std::ofstream outFile;
        vp8codec.Decode(img, yv12Image);
        outFile.open("receiver_output" + std::to_string(i) + ".yv12", std::ios::binary | std::ios::trunc);
        outFile.write((char*)yv12Image.data, yv12Image.size);
        outFile.close();
        delete[] yv12Image.data;
    }
    vp8codec.DeinitDecodeContext();
    return 0;
}
