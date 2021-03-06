#pragma once

#include "DataProcessor.h"
#include <memory>
#include <cstdint>
#include <string>

class IClient: public std::enable_shared_from_this<IClient>
{
public:
    virtual void StartAsync() = 0;
    virtual void HandleEvents() = 0;
    virtual void Destroy() = 0;

    inline void SetStreamId(const std::uint32_t streamId) { m_streamId = streamId; };
    inline void SetWidth(const int width) { m_width = width; };
    inline void SetHeight(const int height) { m_height = height; };
    inline void SetGopSize(const int gopSize) { m_gopSize = gopSize; };
    inline void SetBitrate(const int bitrate) { m_bitrate = bitrate; };

    inline void SetServerIp(const std::string& serverIp) { m_serverIp = serverIp; };
    inline void SetServerTcpPort(const std::uint16_t& serverTcpPort) { m_serverTcpPort = serverTcpPort; };
    
    inline void SetLocalIp(const std::string& localUdpIp) { m_localIp = localUdpIp; };
    inline void SetLocalUdpPort(const std::uint16_t& localUdpPort) { m_localUdpPort = localUdpPort; };

    inline void SetDisableAudio(const bool disableAudio) { m_disableAudio = disableAudio; };
    inline void SetDisableVideo(const bool disableVideo) { m_disableVideo = disableVideo; };

    inline void SetSaveRawStream(const bool saveRawStream) { m_saveRawStream = saveRawStream; };
    inline void SetRawStreamDir(const std::string& rawStreamDir) { m_rawStreamDir = rawStreamDir; };
    inline void SetRawStreamFilename(const std::string& rawStreamFilename) { m_rawStreamFilename = rawStreamFilename; };

protected:
    std::uint32_t m_streamId;
    int m_width, m_height, m_gopSize, m_bitrate;

    std::string m_serverIp;
    std::uint16_t m_serverTcpPort;

    std::string m_localIp;
    std::uint16_t m_localUdpPort;

    bool m_disableAudio;
    bool m_disableVideo;

    bool m_saveRawStream;
    std::string m_rawStreamDir;
    std::string m_rawStreamFilename;
};
