#include "Room.h"
#include "Network.h"
#include "Logger.h"
#include <functional>

Room::Room(std::uint32_t id, boost::asio::io_context& ioContext, 
    const udp_endpoint_t& localUdpEndpoint, const udp_endpoint_t& streamerUdpEndpoint
)
: m_ioContext(ioContext)
, m_id(id)
, m_localUdpEndpoint(localUdpEndpoint)
, m_streamerUdpEndpoint(streamerUdpEndpoint)
, m_isStarted(false)
{
}

void Room::Start()
{
    m_isStarted = true;
    m_localUdpSocket = std::make_shared<boost::asio::ip::udp::socket>(m_ioContext);
    m_localUdpSocket->open(m_localUdpEndpoint.protocol());
    m_localUdpSocket->bind(m_localUdpEndpoint);
    ReadMediaPacket();
}

void Room::Destroy()
{
    if (m_localUdpSocket->is_open())
    {
        boost::system::error_code ec;
        m_localUdpSocket->shutdown(boost::asio::socket_base::shutdown_both, ec);
        m_localUdpSocket->close();
    }
}

void Room::ReadMediaPacket()
{
    std::shared_ptr<char[]> buffer = std::shared_ptr<char[]>(new char[Network::MaxUdpPacketSize]); // store it as member
    m_localUdpSocket->async_receive_from(boost::asio::buffer(&buffer[0], Network::MaxUdpPacketSize), m_streamerUdpEndpoint, 
        [this, that = shared_from_this(), buffer](const boost::system::error_code& ec, std::size_t bytesTransferred){
            if (!ec)
            {
                // LOG_EX_INFO("Read media packet: size(bytes) = " + std::to_string(bytesTransferred));
                Multicast(buffer);
                ReadMediaPacket();              
            } else {
                LOG_EX_WARN_WITH_CONTEXT("Unable to receive media packet: " + ec.message());
            }
        });
}

void Room::Multicast(const std::shared_ptr<char[]>& buffer)
{
    std::lock_guard<std::mutex> lock(m_receiversMutex);
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
                // LOG_EX_INFO("Send media packet: size(bytes) = " + std::to_string(bytesTransferred));
            } else {
                LOG_EX_WARN_WITH_CONTEXT("Unable to send media packet: " + ec.message());
            }
        });
}

void Room::Join(const udp_endpoint_t& receiverEndpoint)
{
    std::lock_guard<std::mutex> lock(m_receiversMutex);
    m_receivers.insert(receiverEndpoint);
}

void Room::Leave(const udp_endpoint_t& receiverEndpoint)
{
    std::lock_guard<std::mutex> lock(m_receiversMutex);
    m_receivers.erase(receiverEndpoint);
}
