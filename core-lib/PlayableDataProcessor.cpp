#include "PlayableDataProcessor.h"

int PlayableDataProcessor::Play(int count)
{
    auto pkt = std::make_shared<media_packet_t>();
    int i = 0; 
    if (count) {               
        for (; i < count; i++)
        {
            Process(pkt);
        }
    } else {
        while (!stop)
        {
            Process(pkt);
            i++;
        }
    }
    return i;
}
