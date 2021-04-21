#pragma once

#include "MediaPacket.h"
#include "rtp/RTPVp8Depay.h"
#include <boost/asio.hpp>
#include <memory>
#include <cstdint>
#include <string>
#include <queue>
#include <mutex>

class ReceiverSession : public std::enable_shared_from_this<ReceiverSession>
{
public:
    enum class State {
        INITIALIZED,
        CONNECTED,
        STOPPED,
    };
    
public:
    ReceiverSession(boost::asio::io_context& ioContext);
    void ConnectToStream(const std::uint32_t id);
    void ReceiveData();

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
    inline void SetFPS(const int fps) { m_fps = fps; };
    
private:
    std::string EncodeLocalAddress();

    boost::asio::io_context& m_ioContext;
    boost::asio::ip::tcp::endpoint m_serverTcpEndpoint;
    boost::asio::ip::udp::endpoint m_serverUdpEndpoint;
    boost::asio::ip::udp::endpoint m_localUdpEndpoint;
    std::string m_localUdpIp;
    std::uint16_t m_localUdpPort;
    std::shared_ptr<boost::asio::ip::tcp::socket> m_tcpSocket;
    std::shared_ptr<boost::asio::ip::udp::socket> m_udpSocket;
    int m_fps;
    State m_state;
    RTPVp8Depay m_rtpVp8Depay;
};
