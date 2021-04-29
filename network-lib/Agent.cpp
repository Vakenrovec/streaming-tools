#include "Agent.h"
#include "Logger.h"
#include "MediaPacket.h"
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
            LOG_EX_WARN("async_accept failed: " + ec.message());
        }
        else
        {
            LOG_EX_WARN("async_accept failed (aborted): " + ec.message());
        }
    });
}

void Agent::ReadNetPacket(std::shared_ptr<boost::asio::ip::tcp::socket> socket)
{
    auto pkt = std::make_shared<net_packet_ptr::element_type>();
    boost::asio::async_read(*socket, boost::asio::buffer(&pkt->header, sizeof(pkt->header)), 
        [this, that = shared_from_this(), socket, pkt](const boost::system::error_code& ec, std::size_t bytesTransferred){
            if (!ec) {
                pkt->data.resize(pkt->header.size);
                boost::asio::async_read(*socket, boost::asio::buffer(const_cast<char*>(pkt->data.data()), pkt->data.size()), 
                    [this, that, socket, pkt](const boost::system::error_code& ec, std::size_t bytesTransferred){
                        if (!ec) {
                            ProcessNetPacket(pkt);
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

void Agent::ProcessNetPacket(net_packet_ptr pkt)
{
    switch (pkt->header.type)
    {
    case NetPacketType::CREATE:
    {
        if (m_rooms->find(pkt->header.id) == m_rooms->end())
        {
            auto address = std::string(&pkt->data[0], pkt->header.size);
            boost::asio::ip::udp::endpoint streamerUdpEndpoint = DecodeAddress(address);
            auto room = std::make_shared<room_ptr::element_type>(m_ioContext, m_localUdpEndpoint, streamerUdpEndpoint);
            m_rooms->insert({ 
                pkt->header.id, 
                room
            });
            room->Start();
            LOG_EX_INFO("Accepted new streamer");
        } 
        break;
    }
    case NetPacketType::DESTROY:
    {
        if (m_rooms->find(pkt->header.id) != m_rooms->end())
        {
            m_rooms->erase(pkt->header.id);
        } 
        break;
    }
    case NetPacketType::CONNECT:
    {
        auto room = m_rooms->find(pkt->header.id);
        if (room != m_rooms->end())
        {
            auto address = std::string(&pkt->data[0], pkt->header.size);
            boost::asio::ip::udp::endpoint endpoint = DecodeAddress(address);
            room->second->Join(endpoint);
            LOG_EX_INFO("Accepted new receiver");
        } 
        break;
    }
    case NetPacketType::DISCONNECT:
    {
        auto room = m_rooms->find(pkt->header.id);
        if (room != m_rooms->end())
        {
            auto address = std::string(&pkt->data[0], pkt->header.size);
            boost::asio::ip::udp::endpoint endpoint = DecodeAddress(address);
            room->second->Leave(endpoint);
        } 
        break;
    }
    default:
        break;
    }
}

boost::asio::ip::udp::endpoint Agent::DecodeAddress(std::string& address)
{
    std::string ip = address.substr(0, address.find(':', 0));
    std::string port = address.substr(address.find(':', 0) + 1);
    return boost::asio::ip::udp::endpoint(boost::asio::ip::address_v4::from_string(ip), std::atoi(port.c_str()));
}
