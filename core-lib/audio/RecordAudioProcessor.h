#pragma once

#include "DataProcessor.h"
#include "MediaPacket.h"
#include <SDL2/SDL.h>
#include <memory>
#include <cstdint>
#include <boost/circular_buffer.hpp>

class RecordAudioProcessor: public DataProcessor
{
public:
    RecordAudioProcessor(std::shared_ptr<boost::circular_buffer<media_packet_ptr>>& circularBuffer);

    void Init() override;
    void Destroy() override;

    void Play();
    void Process(const media_packet_ptr& pkt) override;

private:
    static void AudioRecordingCallback(void* userdata, std::uint8_t* stream, int len);

private:
    std::shared_ptr<boost::circular_buffer<media_packet_ptr>> m_circularBuffer;

    SDL_AudioDeviceID m_recordingDeviceId;
    const int MAX_RECORDING_DEVICES = 10;
    const int MAX_RECORDING_SECONDS = 2;
    const int RECORDING_BUFFER_SECONDS = MAX_RECORDING_SECONDS + 1;
    SDL_AudioSpec m_receivedRecordingSpec;
    std::uint8_t* m_recordingBuffer = nullptr;
    std::uint32_t m_bufferByteSize = 0;
    std::uint32_t m_bufferBytePosition = 0;
    std::uint32_t m_bufferByteMaxPosition = 0;
};
