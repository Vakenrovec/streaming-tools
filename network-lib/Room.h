#pragma once

#include "MediaPacket.h"
#include <boost/asio.hpp>
#include <memory>
#include <cstdint>
#include <string>
#include <set>

using udp_endpoint_t = boost::asio::ip::udp::endpoint;

class Room : public std::enable_shared_from_this<Room>
{
public:
    Room(boost::asio::io_context& ioContext, 
        const udp_endpoint_t& serverUdpEndpoint, const udp_endpoint_t& streamerUdpEndpoint
    );
    void Start();
    void Join(const udp_endpoint_t& receiverEndpoint);
    void Leave(const udp_endpoint_t& receiverEndpoint);

private:
    void ReadMediaPacket();
    void Multicast(const std::shared_ptr<char[]>& buffer);
    void WriteMediaPacket(udp_endpoint_t receiverUdpEndpoint, const std::shared_ptr<char[]>& buffer);

    boost::asio::io_context& m_ioContext;
    std::shared_ptr<boost::asio::ip::udp::socket> m_localUdpSocket;
    udp_endpoint_t m_serverUdpEndpoint;    
    udp_endpoint_t m_streamerUdpEndpoint;
    std::set<udp_endpoint_t> m_receivers;
};

using room_ptr = std::shared_ptr<Room>;
