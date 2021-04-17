#include "StreamerSession.h"
#include "NetPacket.h"
#include "Logger.h"

StreamerSession::StreamerSession(boost::asio::io_context& ioContext)
: m_ioContext(ioContext)
, m_tcpSocket(std::make_shared<boost::asio::ip::tcp::socket>(ioContext))
, m_udpSocket(std::make_shared<boost::asio::ip::udp::socket>(ioContext))
, m_state(State::INITIALIZED)
{
}

void StreamerSession::CreateStream(const std::uint32_t id)
{
    m_tcpSocket->async_connect(m_serverTcpEndpoint, [this, that = shared_from_this(), id](const boost::system::error_code& ec){
        if (!ec) {
            auto pkt = std::make_shared<net_packet_ptr::element_type>();
            pkt->header.type = net_packet_type_t::CREATE;
            pkt->header.id = id;
            pkt->data = EncodeLocalAddress();
            pkt->header.size = pkt->data.size();
            boost::asio::async_write(*m_tcpSocket, boost::asio::buffer(&pkt->header, sizeof(pkt->header)), 
                [this, that, pkt](const boost::system::error_code& ec, std::size_t bytesTransferred){
                    if (!ec) {
                        boost::asio::async_write(*m_tcpSocket, boost::asio::buffer(pkt->data.data(), pkt->data.size()), 
                            [this, that, pkt](const boost::system::error_code& ec, std::size_t bytesTransferred){
                                if (!ec) {
                                    boost::system::error_code ec;
                                    m_tcpSocket->shutdown(boost::asio::socket_base::shutdown_both, ec);
                                    m_tcpSocket->close(ec);
                                    m_udpSocket->open(m_localUdpEndpoint.protocol());
                                    m_udpSocket->bind(m_localUdpEndpoint);
                                    m_state = State::CONNECTED;
                                } else {
                                    LOG_EX_WARN("Unable to write net packet data: " + ec.message());
                                }
                            });
                    } else {
                        LOG_EX_WARN("Unable to write net packet header: " + ec.message());
                    }
                });
        } else {
            LOG_EX_WARN("Unable to connect to server: " + ec.message());
        }
    });
}

std::string StreamerSession::EncodeLocalAddress()
{
    // return std::string(m_localUdpIp + ":" +  std::to_string(m_localUdpPort));
    return std::string(m_localUdpEndpoint.address().to_string() + ":" +  std::to_string(m_localUdpEndpoint.port()));
}

void StreamerSession::WriteData(const media_packet_ptr& pkt)
{
    if (m_state == State::CONNECTED)
    {
        m_udpSocket->async_send_to(boost::asio::buffer(pkt.get(), sizeof(pkt->header) + pkt->header.size), m_serverUdpEndpoint, 
            [this, that = shared_from_this(), pkt](const boost::system::error_code& ec, std::size_t bytesTransferred){
                if (!ec) {

                } else {
                    LOG_EX_WARN("Unable to send media packet: " + ec.message());
                }
            });
    }
}
