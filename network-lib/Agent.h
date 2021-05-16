#pragma once

#include "NetPacket.h"
#include "Room.h"
#include <boost/asio.hpp>
#include <cstdint>
#include <memory>
#include <string>
#include <map>
#include <mutex>

class Agent : public std::enable_shared_from_this<Agent>
{
public:
    Agent(boost::asio::io_context& ioContext);
    void Start();

    inline void SetTcpEndpoint(const std::string& bindTcpIp, const std::uint16_t bindTcpPort) { 
        m_localTcpEndpoint = boost::asio::ip::tcp::endpoint(boost::asio::ip::address_v4::from_string(bindTcpIp), bindTcpPort);
    };
    inline void SetUdpEndpoint(const std::string& bindUdpIp, const std::uint16_t bindUdpPort) { 
        m_localUdpEndpoint = boost::asio::ip::udp::endpoint(boost::asio::ip::address_v4::from_string(bindUdpIp), bindUdpPort); 
    };

private:
    void AcceptNewConnection();
    void ReadNetPacket(const std::shared_ptr<boost::asio::ip::tcp::socket>& socket);
    void ProcessNetPacket(net_packet_ptr pkt, const std::shared_ptr<boost::asio::ip::tcp::socket>& socket);

    boost::asio::io_context& m_ioContext;
    boost::asio::ip::tcp::acceptor m_acceptor;
    boost::asio::ip::tcp::endpoint m_localTcpEndpoint;
    boost::asio::ip::udp::endpoint m_localUdpEndpoint;
    
    std::shared_ptr<std::map<std::uint32_t, room_ptr>> m_rooms;
    std::mutex m_roomsMutex;
};
