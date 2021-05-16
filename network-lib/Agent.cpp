#include "Agent.h"
#include "Logger.h"
#include "MediaPacket.h"
#include "NetworkUtils.h"
#include <functional>

Agent::Agent(boost::asio::io_context& ioContext)
    : m_ioContext(ioContext)
    , m_acceptor(ioContext)
    , m_rooms(std::make_shared<std::map<std::uint32_t, room_ptr>>())
{
}

void Agent::Start()
{
    m_acceptor.open(boost::asio::ip::tcp::v4());
    m_acceptor.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
    m_acceptor.bind(m_localTcpEndpoint);
    m_acceptor.listen();

    AcceptNewConnection();
}

void Agent::AcceptNewConnection()
{
    auto socket = std::make_shared<boost::asio::ip::tcp::socket>(m_ioContext);
    m_acceptor.async_accept(*socket, [this, that = shared_from_this(), socket](const boost::system::error_code& ec) {
        if (!ec)
        {
            ReadNetPacket(socket);
            AcceptNewConnection();
        }
        else if (ec != boost::asio::error::operation_aborted) {
            AcceptNewConnection();
            LOG_EX_WARN("async_accept failed (aborted): " + ec.message());
        }
        else
        {
            LOG_EX_WARN("async_accept failed: " + ec.message());
        }
    });
}

void Agent::ReadNetPacket(const std::shared_ptr<boost::asio::ip::tcp::socket>& socket)
{
    auto pkt = std::make_shared<net_packet_ptr::element_type>();
    boost::asio::async_read(*socket, boost::asio::buffer(&pkt->header, sizeof(pkt->header)), 
        [this, that = shared_from_this(), socket, pkt](const boost::system::error_code& ec, std::size_t bytesTransferred){
            if (!ec) {
                pkt->data.resize(pkt->header.size);
                boost::asio::async_read(*socket, boost::asio::buffer(const_cast<char*>(pkt->data.data()), pkt->data.size()), 
                    [this, that, socket, pkt](const boost::system::error_code& ec, std::size_t bytesTransferred){
                        if (!ec) {                            
                            ProcessNetPacket(pkt, socket);
                            boost::system::error_code ec;
                            socket->shutdown(boost::asio::socket_base::shutdown_both, ec);
                            socket->close(ec);
                        } else {
                            LOG_EX_WARN("Unable to receive net packet data: " + ec.message());
                        }
                    });
            } else {
                LOG_EX_WARN("Unable to receive net packet header: " + ec.message());
            }
        });
}

void Agent::ProcessNetPacket(net_packet_ptr pkt, const std::shared_ptr<boost::asio::ip::tcp::socket>& socket)
{
    auto address = std::string(&pkt->data[0], pkt->header.size);
    boost::asio::ip::udp::endpoint packetUdpEndpoint = NetworkUtils::DecodeUdpAddress(address);
    const auto clientUdpEndpoint = boost::asio::ip::udp::endpoint(
        socket->remote_endpoint().address(),
        packetUdpEndpoint.port()
    );

    switch (pkt->header.type)
    {
    case NetPacketType::CREATE:
    {
        std::lock_guard<std::mutex> lock(m_roomsMutex);
        if (m_rooms->find(pkt->header.id) == m_rooms->end())
        {
            auto room = std::make_shared<room_ptr::element_type>(m_ioContext, m_localUdpEndpoint, clientUdpEndpoint);
            m_rooms->insert({ 
                pkt->header.id, 
                room
            });
            room->Start();
            LOG_EX_INFO_WITH_CONTEXT("Accepted new streamer, room id = %lu, ip = %s, port = %d", 
                pkt->header.id, clientUdpEndpoint.address().to_string().c_str(), clientUdpEndpoint.port());
        } 
        else
        {
            LOG_EX_INFO_WITH_CONTEXT("there is already such room with id = %lu, ip = %s, port = %d", 
                pkt->header.id, clientUdpEndpoint.address().to_string().c_str(), clientUdpEndpoint.port());
        }
        break;
    }
    case NetPacketType::DESTROY:
    {
        std::lock_guard<std::mutex> lock(m_roomsMutex);
        const auto room = m_rooms->find(pkt->header.id);
        if (room != m_rooms->end())
        {
            room->second->Destroy();
            m_rooms->erase(pkt->header.id);
            LOG_EX_INFO_WITH_CONTEXT("Erased streamer, room id = %lu, ip = %s, port = %d", 
                pkt->header.id, clientUdpEndpoint.address().to_string().c_str(), clientUdpEndpoint.port());
        }
        else
        {
            LOG_EX_INFO_WITH_CONTEXT("there isn't such room with id = %lu, ip = %s, port = %d", 
                pkt->header.id, clientUdpEndpoint.address().to_string().c_str(), clientUdpEndpoint.port());
        }
        break;
    }
    case NetPacketType::CONNECT:
    {
        std::lock_guard<std::mutex> lock(m_roomsMutex);
        auto room = m_rooms->find(pkt->header.id);
        if (room != m_rooms->end())
        {
            room->second->Join(clientUdpEndpoint);
            LOG_EX_INFO_WITH_CONTEXT("Connected new receiver, room id = %lu, ip = %s, port = %d", 
                pkt->header.id, clientUdpEndpoint.address().to_string().c_str(), clientUdpEndpoint.port());
        }
        else
        {
            LOG_EX_INFO_WITH_CONTEXT("there isn't such room with id = %lu, ip = %s, port = %d", 
                pkt->header.id, clientUdpEndpoint.address().to_string().c_str(), clientUdpEndpoint.port());
        }
        break;
    }
    case NetPacketType::DISCONNECT:
    {
        std::lock_guard<std::mutex> lock(m_roomsMutex);
        auto room = m_rooms->find(pkt->header.id);
        if (room != m_rooms->end())
        {
            room->second->Leave(clientUdpEndpoint);
            LOG_EX_INFO_WITH_CONTEXT("Disconnected receiver, room id = %lu, ip = %s, port = %d", 
                pkt->header.id, clientUdpEndpoint.address().to_string().c_str(), clientUdpEndpoint.port());
        }
        else
        {
            LOG_EX_INFO_WITH_CONTEXT("there isn't such room with id = %lu, ip = %s, port = %d", 
                pkt->header.id, clientUdpEndpoint.address().to_string().c_str(), clientUdpEndpoint.port());
        }
        break;
    }
    default:
        break;
    }
}
