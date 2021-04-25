#pragma once

#include "DataProcessor.h"
#include "MediaPacket.h"
#include <SDL2/SDL.h>
#include <memory>
#include <cstdint>
#include <boost/circular_buffer.hpp>

class PlaybackAudioProcessor: public DataProcessor
{
public:
    PlaybackAudioProcessor(std::shared_ptr<boost::circular_buffer<media_packet_ptr>>& circularBuffer);

    void Init() override;
    void Destroy() override;

    void Process(const media_packet_ptr& pkt) override;

private:
    static void AudioPlaybackCallback(void* userdata, std::uint8_t* stream, int len);

private:
    std::shared_ptr<boost::circular_buffer<media_packet_ptr>> m_circularBuffer;

    SDL_AudioDeviceID m_playbackDeviceId;
    const int MAX_RECORDING_DEVICES = 10;
    const int MAX_RECORDING_SECONDS = 2;
    const int RECORDING_BUFFER_SECONDS = MAX_RECORDING_SECONDS + 1;
    SDL_AudioSpec m_receivedPlaybackSpec;
    std::uint8_t* m_playbackBuffer = nullptr;
    std::uint32_t m_bufferByteSize = 0;
    std::uint32_t m_bufferBytePosition = 0;
    std::uint32_t m_bufferByteMaxPosition = 0;
};
