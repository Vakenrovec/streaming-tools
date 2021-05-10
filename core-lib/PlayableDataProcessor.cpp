#include "PlayableDataProcessor.h"

int PlayableDataProcessor::Play(int count)
{    
    int i = 0; 
    // auto pkt = std::make_shared<media_packet_t>();
    if (count) {               
        for (; i < count; i++)
        {
            auto pkt = std::make_shared<media_packet_t>();
            Process(pkt);
        }
    } else {
        while (!stop)
        {
            auto pkt = std::make_shared<media_packet_t>();
            Process(pkt);
            i++;
        }
    }
    return i;
}
