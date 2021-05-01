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
    enum class State {
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

    inline void SetServerTcpEndpoint(const std::string& bindTcpIp, const std::uint16_t bindTcpPort) { 
        m_serverTcpEndpoint = boost::asio::ip::tcp::endpoint(boost::asio::ip::address_v4::from_string(bindTcpIp), bindTcpPort);
    };
    inline void SetServerUdpEndpoint(const std::string& bindUdpIp, const std::uint16_t bindUdpPort) { 
        m_serverUdpEndpoint = boost::asio::ip::udp::endpoint(boost::asio::ip::address_v4::from_string(bindUdpIp), bindUdpPort); 
    };
    inline void SetLocalUdpEndpoint(const std::string& bindUdpIp, const std::uint16_t bindUdpPort) { 
        m_localUdpEndpoint = boost::asio::ip::udp::endpoint(boost::asio::ip::address_v4::from_string(bindUdpIp), bindUdpPort); 
    };
    inline void SetLocalUdpIp(const std::string& localIp) { m_localUdpIp = localIp; };
    inline void SetLocalUdpPort(const std::uint16_t& localPort) { m_localUdpPort = localPort; };
    
private:
    void CreateStream();
    void DestroyStream();
    void SendData(const udp_packet_ptr& pkt);

public:
    boost::asio::io_context& m_ioContext;
    boost::asio::ip::tcp::endpoint m_serverTcpEndpoint;
    boost::asio::ip::udp::endpoint m_serverUdpEndpoint;
    boost::asio::ip::udp::endpoint m_localUdpEndpoint;
    std::string m_localUdpIp;
    std::uint16_t m_localUdpPort;
    std::shared_ptr<boost::asio::ip::tcp::socket> m_tcpSocket;
    std::shared_ptr<boost::asio::ip::udp::socket> m_udpSocket;
    int m_sessionId;
    State m_state;
};
