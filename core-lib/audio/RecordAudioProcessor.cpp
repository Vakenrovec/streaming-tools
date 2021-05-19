#include "RecordAudioProcessor.h"
#include "Logger.h"
#include "DateTimeUtils.h"
#include <string>
#include <thread>
#include <chrono>

RecordAudioProcessor::RecordAudioProcessor()
{
}

void RecordAudioProcessor::AudioRecordingCallback(void* userdata, std::uint8_t* stream, int len)
{
    RecordAudioProcessor *that = (RecordAudioProcessor*)userdata;
    auto pkt = std::make_shared<media_packet_t>();
    pkt->header.type = MediaPacketType::AUDIO_SAMPLES;
    pkt->header.ts = DateTimeUtils::GetCurrentTimeNanoseconds();
    pkt->header.size = len;
    std::copy(stream, stream + len, pkt->data);
    that->Process(pkt);
}

void RecordAudioProcessor::Init()
{
    int gRecordingDeviceCount = SDL_GetNumAudioDevices(true);
    if (gRecordingDeviceCount < 1)
    {
        LOG_EX_WARN_WITH_CONTEXT("Unable to get audio capture device! gRecordingDeviceCount < 1. SDL Error: " + std::string(SDL_GetError()));
        return;
    }
    LOG_EX_INFO("Available audio microphones:");
    for(int i = 0; i < gRecordingDeviceCount; i++)
    {
        LOG_EX_INFO("      %d - %s", i, SDL_GetAudioDeviceName(i, true));
    }

    int index = 0;
    SDL_AudioSpec desiredRecordingSpec;
    SDL_zero(desiredRecordingSpec);
    desiredRecordingSpec.freq = 44100;
    desiredRecordingSpec.format = AUDIO_F32; // AUDIO_F32 AUDIO_S16
    desiredRecordingSpec.channels = 2;
    desiredRecordingSpec.samples = 4096; // 4096 1152;
    desiredRecordingSpec.callback = AudioRecordingCallback;
    desiredRecordingSpec.userdata = this;
    m_recordingDeviceId = SDL_OpenAudioDevice(
        SDL_GetAudioDeviceName(index, true), true, 
        &desiredRecordingSpec, &m_receivedRecordingSpec, SDL_AUDIO_ALLOW_FORMAT_CHANGE
    );
    if (m_recordingDeviceId == 0)
    {
        LOG_EX_WARN_WITH_CONTEXT("Failed to open microphone! SDL Error: " + std::string(SDL_GetError()));
        return;
    }

    int bytesPerSample = m_receivedRecordingSpec.channels * (SDL_AUDIO_BITSIZE(m_receivedRecordingSpec.format) / 8);
    int bytesPerSecond = m_receivedRecordingSpec.freq * bytesPerSample;
    m_bufferByteSize = RECORDING_BUFFER_SECONDS * bytesPerSecond;
    m_bufferByteMaxPosition = MAX_RECORDING_SECONDS * bytesPerSecond;
    m_bufferByteSize = bytesPerSample * m_receivedRecordingSpec.samples;
    SDL_PauseAudioDevice(m_recordingDeviceId, false);
    LOG_EX_INFO("Open audio microphone with id = %lu", m_recordingDeviceId);

    DataProcessor::Init();
}

void RecordAudioProcessor::Destroy()
{
    SDL_LockAudioDevice(m_recordingDeviceId);
    SDL_PauseAudioDevice(m_recordingDeviceId, true);
    SDL_UnlockAudioDevice(m_recordingDeviceId);
    LOG_EX_INFO("Close audio microphone with id = %lu", m_recordingDeviceId);

    DataProcessor::Destroy();
}
