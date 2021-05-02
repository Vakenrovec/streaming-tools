#pragma once

#include "DataProcessor.h"
#include <memory>
#include <cstdint>
#include <boost/asio.hpp>
#include <string>
#include <thread>

class Receiver: public std::enable_shared_from_this<Receiver>
{
public:
    Receiver();

    void Start();
    void HandleEvents();
    void Destroy();

    inline void SetStreamId(std::uint32_t streamId) { m_streamId = streamId; };
    inline void SetWidth(int width) { m_width = width; };
    inline void SetHeight(int height) { m_height = height; };
    inline void SetGopSize(int gopSize) { m_gopSize = gopSize; };
    inline void SetBitrate(int bitrate) { m_bitrate = bitrate; };

    inline void SetServerTcpIp(const std::string& serverTcpIp) { m_serverTcpIp = serverTcpIp; };
    inline void SetServerTcpPort(const std::uint16_t& serverTcpPort) { m_serverTcpPort = serverTcpPort; };
    inline void SetServerUdpIp(const std::string& serverUdpIp) { m_serverUdpIp = serverUdpIp; };
    inline void SetServerUdpPort(const std::uint16_t& serverUdpPort) { m_serverUdpPort = serverUdpPort; };
    inline void SetLocalUdpIp(const std::string& localUdpIp) { m_localUdpIp = localUdpIp; };
    inline void SetLocalUdpPort(const std::uint16_t& localUdpPort) { m_localUdpPort = localUdpPort; };

private:
    std::uint32_t m_streamId;
    int m_width, m_height, m_gopSize, m_bitrate;

    std::string m_serverTcpIp;
    std::uint16_t m_serverTcpPort;
    std::string m_serverUdpIp;
    std::uint16_t m_serverUdpPort;
    std::string m_localUdpIp;
    std::uint16_t m_localUdpPort;

    std::shared_ptr<boost::asio::io_context> m_ioVideoContext, m_ioAudioContext;
    std::shared_ptr<boost::asio::io_context::work> m_videoWork, m_audioWork;
    std::shared_ptr<std::thread> m_videoThread, m_audioThread;

    std::shared_ptr<DataProcessor> m_firstProcessor, m_lastProcessor;
};
