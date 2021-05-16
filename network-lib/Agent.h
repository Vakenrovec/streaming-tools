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

    inline void SetIp(const std::string& serverUdpIp) { m_localIp = serverUdpIp; };
    inline void SetTcpPort(const std::uint16_t& serverTcpPort) { m_localTcpPort = serverTcpPort; };
    inline void SetUdpPort(const std::uint16_t& serverUdpPort) { m_localUdpPort = serverUdpPort; };

private:
    void AcceptNewConnection();
    void ReadNetPacket(const std::shared_ptr<boost::asio::ip::tcp::socket>& socket);
    room_ptr ProcessNetPacket(net_packet_ptr pkt, const std::shared_ptr<boost::asio::ip::tcp::socket>& socket);
    inline std::uint16_t GetFreeUdpPort() { return m_localUdpPort++; };

    boost::asio::io_context& m_ioContext;
    boost::asio::ip::tcp::acceptor m_acceptor;

    std::string m_localIp;
    std::uint16_t m_localTcpPort, m_localUdpPort;
    boost::asio::ip::tcp::endpoint m_localTcpEndpoint;
    
    std::shared_ptr<std::map<std::uint32_t, room_ptr>> m_rooms;
    std::mutex m_roomsMutex;
};
