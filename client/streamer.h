#pragma once

#include "DataProcessor.h"
#include <memory>
#include <cstdint>
#include <boost/asio.hpp>
#include <string>
#include <thread>

class Streamer: public std::enable_shared_from_this<Streamer>
{
public:
    Streamer();

    void StartAsync();
    void HandleEvents();
    void Destroy();

    inline void SetStreamId(const std::uint32_t streamId) { m_streamId = streamId; };
    inline void SetWidth(const int width) { m_width = width; };
    inline void SetHeight(const int height) { m_height = height; };
    inline void SetGopSize(const int gopSize) { m_gopSize = gopSize; };
    inline void SetBitrate(const int bitrate) { m_bitrate = bitrate; };

    inline void SetServerTcpIp(const std::string& serverTcpIp) { m_serverTcpIp = serverTcpIp; };
    inline void SetServerTcpPort(const std::uint16_t& serverTcpPort) { m_serverTcpPort = serverTcpPort; };
    inline void SetServerUdpIp(const std::string& serverUdpIp) { m_serverUdpIp = serverUdpIp; };
    inline void SetServerUdpPort(const std::uint16_t& serverUdpPort) { m_serverUdpPort = serverUdpPort; };
    inline void SetLocalUdpIp(const std::string& localUdpIp) { m_localUdpIp = localUdpIp; };
    inline void SetLocalUdpPort(const std::uint16_t& localUdpPort) { m_localUdpPort = localUdpPort; };

    inline void SetDisableAudio(const bool disableAudio) { m_disableAudio = disableAudio; };
    inline void SetDisableVideo(const bool disableVideo) { m_disableVideo = disableVideo; };

    inline void SetSaveRawStream(const bool saveRawStream) { m_saveRawStream = saveRawStream; };
    inline void SetRawStreamDir(const std::string& rawStreamsDir) { m_rawStreamsDir = rawStreamsDir; };

private:
    std::uint32_t m_streamId;
    int m_width, m_height, m_gopSize, m_bitrate;

    std::string m_serverTcpIp;
    std::uint16_t m_serverTcpPort;
    std::string m_serverUdpIp;
    std::uint16_t m_serverUdpPort;
    std::string m_localUdpIp;
    std::uint16_t m_localUdpPort;

    bool m_disableAudio;
    bool m_disableVideo;

    bool m_saveRawStream;
    std::string m_rawStreamsDir;

private:
    std::shared_ptr<boost::asio::io_context> m_ioContext;
    std::shared_ptr<boost::asio::io_context::work> m_work;
    std::shared_ptr<std::thread> m_pipelinePlayThread, m_videoSenderThread;

    std::shared_ptr<DataProcessor> m_firstVideoProcessor, m_firstAudioProcessor;
};
