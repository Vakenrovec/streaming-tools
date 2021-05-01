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
    RecordAudioProcessor();

    void Init() override;
    void Destroy() override;

private:
    static void AudioRecordingCallback(void* userdata, std::uint8_t* stream, int len);

private:
    SDL_AudioDeviceID m_recordingDeviceId;
    SDL_AudioSpec m_receivedRecordingSpec;

    const int MAX_RECORDING_DEVICES = 10;
    const int MAX_RECORDING_SECONDS = 2;
    const int RECORDING_BUFFER_SECONDS = MAX_RECORDING_SECONDS + 1;

    std::uint8_t* m_recordingBuffer = nullptr;
    std::uint32_t m_bufferByteSize = 0;
    std::uint32_t m_bufferBytePosition = 0;
    std::uint32_t m_bufferByteMaxPosition = 0;
};
