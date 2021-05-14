#include "PlaybackAudioProcessor.h"
#include "Logger.h"
#include <string>

PlaybackAudioProcessor::PlaybackAudioProcessor(int audioBufferElements)
{
    m_circularBuffer = std::make_shared<boost::circular_buffer<media_packet_ptr>>(audioBufferElements);
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
    std::copy(pkt->data, pkt->data + len, stream);
}

void PlaybackAudioProcessor::Init()
{
    int gPlaybackDeviceCount = SDL_GetNumAudioDevices(false);
    if (gPlaybackDeviceCount < 1)
    {
        LOG_EX_WARN_WITH_CONTEXT("Unable to get audio playback device! SDL Error: " + std::string(SDL_GetError()));
        return;
    }
    LOG_EX_INFO_WITH_CONTEXT("Available audio playback devices:");
    for(int i = 0; i < gPlaybackDeviceCount; i++)
    {
        LOG_EX_INFO_WITH_CONTEXT("      %d - %s", i, SDL_GetAudioDeviceName(i, false));
    }

    int index = 0;
    SDL_AudioSpec desiredPlaybackSpec;
    SDL_zero(desiredPlaybackSpec);
    desiredPlaybackSpec.freq = 44100;
    desiredPlaybackSpec.format = AUDIO_F32; // AUDIO_F32 AUDIO_S16
    desiredPlaybackSpec.channels = 2;
    desiredPlaybackSpec.samples = 4096; // 4096 1152;
    desiredPlaybackSpec.callback = AudioPlaybackCallback;
    desiredPlaybackSpec.userdata = this;
    m_playbackDeviceId = SDL_OpenAudioDevice(
        SDL_GetAudioDeviceName(index, false), false, 
        &desiredPlaybackSpec, &m_receivedPlaybackSpec, SDL_AUDIO_ALLOW_FORMAT_CHANGE
    );
    if(m_playbackDeviceId == 0)
    {
        LOG_EX_WARN_WITH_CONTEXT("Failed to open playback device! SDL Error: %s", SDL_GetError());
        return;
    }

    int bytesPerSample = m_receivedPlaybackSpec.channels * (SDL_AUDIO_BITSIZE(m_receivedPlaybackSpec.format) / 8);
    int bytesPerSecond = m_receivedPlaybackSpec.freq * bytesPerSample;
    m_bufferByteSize = RECORDING_BUFFER_SECONDS * bytesPerSecond;
    m_bufferByteMaxPosition = MAX_RECORDING_SECONDS * bytesPerSecond;
    m_bufferByteSize = bytesPerSample * m_receivedPlaybackSpec.samples;
    SDL_PauseAudioDevice(m_playbackDeviceId, false);
    LOG_EX_INFO_WITH_CONTEXT("Open playback device with id = %lu", m_playbackDeviceId);

    DataProcessor::Init();
}

void PlaybackAudioProcessor::Destroy()
{
    SDL_LockAudioDevice(m_playbackDeviceId);
    SDL_PauseAudioDevice(m_playbackDeviceId, true);
    SDL_UnlockAudioDevice(m_playbackDeviceId);
    LOG_EX_INFO_WITH_CONTEXT("Close audio playback with id = %lu", m_playbackDeviceId);

    DataProcessor::Destroy();
}

void PlaybackAudioProcessor::Process(const media_packet_ptr& pkt)
{
    if (pkt->header.type == MediaPacketType::AUDIO_SAMPLES)
    {
        m_circularBuffer->push_back(pkt);
        DataProcessor::Process(pkt);
    }
    else
    {
        LOG_EX_WARN_WITH_CONTEXT("Incorrect packet type: %d", pkt->header.type);
    }
}
