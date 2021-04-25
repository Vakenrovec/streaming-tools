#include "Room.h"
#include "Network.h"
#include "Logger.h"
#include <functional>

Room::Room(boost::asio::io_context& ioContext, 
    const udp_endpoint_t& serverUdpEndpoint, const udp_endpoint_t& streamerUdpEndpoint
)
: m_ioContext(ioContext)
, m_localUdpSocket(std::make_shared<boost::asio::ip::udp::socket>(ioContext, serverUdpEndpoint))
, m_serverUdpEndpoint(serverUdpEndpoint)
, m_streamerUdpEndpoint(streamerUdpEndpoint)
// , m_receivers(std::make_shared<>())
{
}

void Room::Start()
{
    ReadMediaPacket();
}

void Room::ReadMediaPacket()
{
    std::shared_ptr<char[]> buffer = std::shared_ptr<char[]>(new char[Network::MaxUdpPacketSize]); // store it as member
    m_localUdpSocket->async_receive_from(boost::asio::buffer(&buffer[0], Network::MaxUdpPacketSize), m_streamerUdpEndpoint, 
        [this, that = shared_from_this(), buffer](const boost::system::error_code& ec, std::size_t bytesTransferred){
            if (!ec)
            {
                LOG_EX_INFO("Read media packet: size(bytes) = " + std::to_string(bytesTransferred));
                Multicast(buffer);
                ReadMediaPacket();              
            } else {
                LOG_EX_WARN("Unable to receive media packet: " + ec.message());
            }
        });
}

void Room::Multicast(const std::shared_ptr<char[]>& buffer)
{
    std::for_each(m_receivers.begin(), m_receivers.end(), 
        std::bind(&Room::WriteMediaPacket, shared_from_this(), std::placeholders::_1, buffer)    
    );
}

void Room::WriteMediaPacket(udp_endpoint_t receiverUdpEndpoint, const std::shared_ptr<char[]>& buffer)
{
    auto port = receiverUdpEndpoint.port();
    auto ip = receiverUdpEndpoint.address().to_string();
    m_localUdpSocket->async_send_to(boost::asio::buffer(buffer.get(), Network::MaxUdpPacketSize), receiverUdpEndpoint, 
        [this, that = shared_from_this(), buffer](const boost::system::error_code& ec, std::size_t bytesTransferred){
            if (!ec) {
                LOG_EX_INFO("Send media packet: size(bytes) = " + std::to_string(bytesTransferred));
            } else {
                LOG_EX_WARN("Unable to send media packet: " + ec.message());
            }
        });
}

void Room::Join(const udp_endpoint_t& receiverEndpoint)
{
    auto port = receiverEndpoint.port();
    auto ip = receiverEndpoint.address().to_string();
    m_receivers.insert(receiverEndpoint);
}

void Room::Leave(const udp_endpoint_t& receiverEndpoint)
{
    m_receivers.erase(receiverEndpoint);
}
