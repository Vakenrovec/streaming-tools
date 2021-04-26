#include "RecordAudioProcessor.h"
#include "Logger.h"
#include <string>

RecordAudioProcessor::RecordAudioProcessor(std::shared_ptr<boost::circular_buffer<media_packet_ptr>>& circularBuffer)
: m_circularBuffer(circularBuffer)
{
}

void RecordAudioProcessor::AudioRecordingCallback(void* userdata, std::uint8_t* stream, int len)
{
    RecordAudioProcessor *that = (RecordAudioProcessor*)userdata;
    auto pkt = std::make_shared<media_packet_t>();
    std::copy(stream, stream + len, pkt->data);
    that->Process(pkt);
    // memcpy(&that->m_recordingBuffer[that->m_bufferBytePosition], stream, len);
    // that->m_bufferBytePosition += len;
}

void RecordAudioProcessor::Init()
{
    int gRecordingDeviceCount = SDL_GetNumAudioDevices(true);
    if (gRecordingDeviceCount < 1)
    {
        LOG_EX_WARN("Unable to get audio capture device! SDL Error: " + std::string(SDL_GetError()));
        return;
    }
    for(int i = 0; i < gRecordingDeviceCount; i++)
    {
        LOG_EX_INFO(std::to_string(i) + ": " + SDL_GetAudioDeviceName(i, true));
    }

    int index = 0;
    SDL_AudioSpec desiredRecordingSpec;
    SDL_zero(desiredRecordingSpec);
    desiredRecordingSpec.freq = 44100;
    desiredRecordingSpec.format = AUDIO_F32;
    desiredRecordingSpec.channels = 2;
    desiredRecordingSpec.samples = 4096;
    desiredRecordingSpec.callback = AudioRecordingCallback;
    desiredRecordingSpec.userdata = this;
    m_recordingDeviceId = SDL_OpenAudioDevice(
        SDL_GetAudioDeviceName(index, true), true, 
        &desiredRecordingSpec, &m_receivedRecordingSpec, SDL_AUDIO_ALLOW_FORMAT_CHANGE
    );
    if (m_recordingDeviceId == 0)
    {
        LOG_EX_WARN("Failed to open recording device! SDL Error: " + std::string(SDL_GetError()));
        return;
    }

    int bytesPerSample = m_receivedRecordingSpec.channels * (SDL_AUDIO_BITSIZE(m_receivedRecordingSpec.format) / 8);
    int bytesPerSecond = m_receivedRecordingSpec.freq * bytesPerSample;
    m_bufferByteSize = RECORDING_BUFFER_SECONDS * bytesPerSecond;
    m_bufferByteMaxPosition = MAX_RECORDING_SECONDS * bytesPerSecond;

    m_bufferByteSize = bytesPerSample * m_receivedRecordingSpec.samples;
    m_recordingBuffer = new std::uint8_t[m_bufferByteSize];
    memset(m_recordingBuffer, 0, m_bufferByteSize);
    m_bufferBytePosition = 0;
    SDL_PauseAudioDevice(m_recordingDeviceId, false);

    DataProcessor::Init();
}

void RecordAudioProcessor::Destroy()
{
    SDL_LockAudioDevice(m_recordingDeviceId);
    SDL_PauseAudioDevice(m_recordingDeviceId, true);
    SDL_UnlockAudioDevice(m_recordingDeviceId);

    if(m_recordingBuffer != nullptr)
    {
        delete[] m_recordingBuffer;
        m_recordingBuffer = nullptr;
    }

    DataProcessor::Destroy();
}

void RecordAudioProcessor::Play()
{

}

void RecordAudioProcessor::Process(const media_packet_ptr& pkt)
{
    DataProcessor::Process(pkt);
}
