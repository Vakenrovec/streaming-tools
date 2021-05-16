#pragma once

#include "MediaPacket.h"
#include <boost/asio.hpp>
#include <memory>
#include <cstdint>
#include <string>
#include <set>
#include <mutex>

using udp_endpoint_t = boost::asio::ip::udp::endpoint;

class Room : public std::enable_shared_from_this<Room>
{
public:
    Room(std::uint32_t id, boost::asio::io_context& ioContext, 
        const udp_endpoint_t& localUdpEndpoint, const udp_endpoint_t& streamerUdpEndpoint
    );
    void Start();
    void Destroy();
    void Join(const udp_endpoint_t& receiverEndpoint);
    void Leave(const udp_endpoint_t& receiverEndpoint);

    inline std::uint16_t GetLocalUdpPort() const { return m_localUdpEndpoint.port(); };
    inline bool IsStarted() const { return m_isStarted; };
    inline std::uint32_t GetId() const { return m_id; };

private:
    void ReadMediaPacket();
    void Multicast(const std::shared_ptr<char[]>& buffer);
    void WriteMediaPacket(udp_endpoint_t receiverUdpEndpoint, const std::shared_ptr<char[]>& buffer);

    boost::asio::io_context& m_ioContext;
    udp_endpoint_t m_localUdpEndpoint, m_streamerUdpEndpoint;
    std::shared_ptr<boost::asio::ip::udp::socket> m_localUdpSocket;

    std::uint32_t m_id;
    bool m_isStarted;

    std::set<udp_endpoint_t> m_receivers;
    std::mutex m_receiversMutex;
};

using room_ptr = std::shared_ptr<Room>;
