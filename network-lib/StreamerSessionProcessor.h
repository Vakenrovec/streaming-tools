#pragma once

#include "DataProcessor.h"
#include "UDPPacket.h"
#include <boost/asio.hpp>
#include <memory>
#include <cstdint>
#include <string>

class StreamerSessionProcessor: public DataProcessor
{
public:
    enum class StreamerSessionState {
        INITIALIZED,
        SESSION_CREATED,
        SESSION_DESTROYED,
        STOPPED,
    };

public:
    StreamerSessionProcessor(boost::asio::io_context& ioContext, const std::uint32_t sessionId);

    void Init() override;
    void Destroy() override;

    void Process(const udp_packet_ptr& pkt) override;

    inline void SetLocalIp(const std::string& localIp) { m_localIp = localIp; };
    inline void SetServerIp(const std::string& serverIp) { m_serverIp = serverIp; };
    inline void SetLocalUdpPort(const std::uint16_t& localPort) { m_localUdpPort = localPort; };
    inline void SetServerTcpPort(const std::uint16_t& serverPort) { m_serverTcpPort = serverPort; };

    inline void SetSessionState(const StreamerSessionState sessionState) { m_sessionState = sessionState; };    // for tests
    inline StreamerSessionState GetSessionState() const { return m_sessionState; };                                           
    inline std::shared_ptr<boost::asio::ip::udp::socket> GetUdpSocket() const { return m_udpSocket; };          // for tests
    inline boost::asio::ip::udp::endpoint GetLocalUdpEndpoint() const { return m_localUdpEndpoint; };           // for tests

private:
    void CreateStream();
    void DestroyStream();
    void SendData(const udp_packet_ptr& pkt);

    boost::asio::io_context& m_ioContext;
    std::shared_ptr<boost::asio::ip::tcp::socket> m_tcpSocket;
    std::shared_ptr<boost::asio::ip::udp::socket> m_udpSocket;

    std::string m_localIp, m_serverIp;
    std::uint16_t m_localUdpPort, m_serverTcpPort;
    boost::asio::ip::tcp::endpoint m_serverTcpEndpoint;
    boost::asio::ip::udp::endpoint m_localUdpEndpoint, m_serverUdpEndpoint;
    
    int m_sessionId;
    StreamerSessionState m_sessionState;
};
