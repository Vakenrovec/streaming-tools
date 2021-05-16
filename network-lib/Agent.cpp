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
    m_localTcpEndpoint = boost::asio::ip::tcp::endpoint(
        boost::asio::ip::address_v4::from_string(m_localIp), m_localTcpPort
    );
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
                            room_ptr room = ProcessNetPacket(pkt, socket);
                            if (room)
                            {
                                std::uint16_t roomUdpPort = room->GetLocalUdpPort();
                                std::shared_ptr<char[]> roomUdpPortBuffer = std::shared_ptr<char[]>(new char[sizeof(std::uint16_t)]);
                                std::copy(static_cast<const char*>(static_cast<const void*>(&roomUdpPort)),
                                    static_cast<const char*>(static_cast<const void*>(&roomUdpPort)) + sizeof(std::uint16_t),
                                    &roomUdpPortBuffer[0]
                                );
                                boost::asio::async_write(*socket, boost::asio::buffer(&roomUdpPortBuffer[0], sizeof(std::uint16_t)), 
                                    [this, that, socket, room](const boost::system::error_code& ec, std::size_t bytesTransferred){
                                        boost::system::error_code dummy;
                                        socket->shutdown(boost::asio::socket_base::shutdown_both, dummy);
                                        socket->close(dummy);
                                        if (!ec) {
                                            if (!room->IsStarted()) {
                                                room->Start();
                                                LOG_EX_INFO("Streamer started room on port %d with id = %lu", room->GetLocalUdpPort(), room->GetId());
                                            } else {
                                                LOG_EX_INFO("Receiver connected to room with id = %lu", room->GetId());
                                            }                                         
                                        } else {
                                            LOG_EX_WARN_WITH_CONTEXT("Unable to write room port");
                                        }                                        
                                    });
                            }
                            else
                            {
                                boost::system::error_code ec;
                                socket->shutdown(boost::asio::socket_base::shutdown_both, ec);
                                socket->close(ec);
                            }
                        } else {
                            LOG_EX_WARN_WITH_CONTEXT("Unable to receive net packet data: " + ec.message());
                        }
                    });
            } else {
                LOG_EX_WARN_WITH_CONTEXT("Unable to receive net packet header: " + ec.message());
            }
        });
}

room_ptr Agent::ProcessNetPacket(net_packet_ptr pkt, const std::shared_ptr<boost::asio::ip::tcp::socket>& socket)
{
    room_ptr result = nullptr;

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
            const auto roomUdpEndpoint = boost::asio::ip::udp::endpoint(
                boost::asio::ip::address::from_string(m_localIp),
                GetFreeUdpPort()
            );
            const auto room = std::make_shared<room_ptr::element_type>(
                pkt->header.id, m_ioContext, roomUdpEndpoint, clientUdpEndpoint
            );
            m_rooms->insert({ 
                pkt->header.id, 
                room
            });
            result = room;
            LOG_EX_INFO("Accepted new streamer, room id = %lu. Client: ip = %s, port = %d", 
                pkt->header.id, clientUdpEndpoint.address().to_string().c_str(), clientUdpEndpoint.port());
        } 
        else
        {
            LOG_EX_INFO_WITH_CONTEXT("There is already such room with id = %lu. Client: ip = %s, port = %d", 
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
            LOG_EX_INFO("Erased streamer, room id = %lu. Client: ip = %s, port = %d", 
                pkt->header.id, clientUdpEndpoint.address().to_string().c_str(), clientUdpEndpoint.port());
        }
        else
        {
            LOG_EX_INFO_WITH_CONTEXT("There isn't such room with id = %lu. Client: ip = %s, port = %d", 
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
            result = room->second;
            LOG_EX_INFO("Connected new receiver, room id = %lu. Client: ip = %s, port = %d", 
                pkt->header.id, clientUdpEndpoint.address().to_string().c_str(), clientUdpEndpoint.port());
        }
        else
        {
            LOG_EX_INFO_WITH_CONTEXT("There isn't such room with id = %lu. Client: ip = %s, port = %d", 
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
            LOG_EX_INFO("Disconnected receiver, room id = %lu. Client: ip = %s, port = %d", 
                pkt->header.id, clientUdpEndpoint.address().to_string().c_str(), clientUdpEndpoint.port());
        }
        else
        {
            LOG_EX_INFO_WITH_CONTEXT("There isn't such room with id = %lu. Client: ip = %s, port = %d", 
                pkt->header.id, clientUdpEndpoint.address().to_string().c_str(), clientUdpEndpoint.port());
        }
        break;
    }
    default:
        break;
    }

    return result;
}
