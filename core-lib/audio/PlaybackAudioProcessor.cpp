#include "PlaybackAudioProcessor.h"
#include "Logger.h"
#include <string>

PlaybackAudioProcessor::PlaybackAudioProcessor(std::shared_ptr<boost::circular_buffer<media_packet_ptr>>& circularBuffer)
: m_circularBuffer(circularBuffer)
{
}

void PlaybackAudioProcessor::AudioPlaybackCallback(void* userdata, std::uint8_t* stream, int len)
{
    PlaybackAudioProcessor *that = (PlaybackAudioProcessor*)userdata;
    media_packet_ptr pkt;
    if (that->m_circularBuffer->empty())
    {
        pkt = std::make_shared<media_packet_t>();
        memset(pkt->data, 0, len);
    }
    else
    {
        pkt = that->m_circularBuffer->front();
        that->m_circularBuffer->pop_front();        
    }
    memcpy(stream, pkt->data, len);

    // memcpy(stream, &m_recordingBuffer[m_bufferBytePosition], len);
    // that->m_bufferBytePosition += len;
}

void PlaybackAudioProcessor::Init()
{
    SDL_AudioSpec desiredPlaybackSpec;
    SDL_zero(desiredPlaybackSpec);
    desiredPlaybackSpec.freq = 44100;
    desiredPlaybackSpec.format = AUDIO_F32;
    desiredPlaybackSpec.channels = 2;
    desiredPlaybackSpec.samples = 4096;
    desiredPlaybackSpec.callback = AudioPlaybackCallback;
    desiredPlaybackSpec.userdata = this;
    m_playbackDeviceId = SDL_OpenAudioDevice(nullptr, false, 
        &desiredPlaybackSpec, &m_receivedPlaybackSpec, SDL_AUDIO_ALLOW_FORMAT_CHANGE
    );
    if(m_playbackDeviceId == 0)
    {
        LOG_EX_WARN("Failed to open playback device! SDL Error: " + std::string(SDL_GetError()));
        return;
    }

    int bytesPerSample = m_receivedPlaybackSpec.channels * (SDL_AUDIO_BITSIZE(m_receivedPlaybackSpec.format) / 8);
    int bytesPerSecond = m_receivedPlaybackSpec.freq * bytesPerSample;
    m_bufferByteSize = RECORDING_BUFFER_SECONDS * bytesPerSecond;
    m_bufferByteMaxPosition = MAX_RECORDING_SECONDS * bytesPerSecond;

    m_bufferByteSize = bytesPerSample * m_receivedPlaybackSpec.samples;
    m_playbackBuffer = new std::uint8_t[m_bufferByteSize];
    memset(m_playbackBuffer, 0, m_bufferByteSize);
    m_bufferBytePosition = 0;
    SDL_PauseAudioDevice(m_playbackDeviceId, false);

    DataProcessor::Init();
}

void PlaybackAudioProcessor::Destroy()
{
    SDL_LockAudioDevice(m_playbackDeviceId);
    SDL_PauseAudioDevice(m_playbackDeviceId, true);
    SDL_UnlockAudioDevice(m_playbackDeviceId);

    if(m_playbackBuffer != nullptr)
    {
        delete[] m_playbackBuffer;
        m_playbackBuffer = nullptr;
    }

    DataProcessor::Destroy();
}

void PlaybackAudioProcessor::Process(const media_packet_ptr& pkt)
{
    m_circularBuffer->push_back(pkt);
    DataProcessor::Process(pkt);
}
