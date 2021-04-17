#include "ReceiverSession.h"
#include "NetPacket.h"
#include "Logger.h"
#include "udp.h"

ReceiverSession::ReceiverSession(boost::asio::io_context& ioContext)
: m_ioContext(ioContext)
, m_tcpSocket(std::make_shared<boost::asio::ip::tcp::socket>(ioContext))
, m_udpSocket(std::make_shared<boost::asio::ip::udp::socket>(ioContext))
, m_state(State::INITIALIZED)
, m_writeQueue()
{
}

void ReceiverSession::ConnectToStream(const std::uint32_t id)
{
    m_tcpSocket->async_connect(m_serverTcpEndpoint, [this, that = shared_from_this(), id](const boost::system::error_code& ec){
        if (!ec) {
            auto pkt = std::make_shared<net_packet_ptr::element_type>();
            pkt->header.type = net_packet_type_t::CONNECT;
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
                                    ReceiveData();
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

std::string ReceiverSession::EncodeLocalAddress()
{
    // return std::string(m_localUdpIp + ":" +  std::to_string(m_localUdpPort));
    return std::string(m_localUdpEndpoint.address().to_string() + ":" +  std::to_string(m_localUdpEndpoint.port()));
}

void ReceiverSession::ReceiveData()
{
    if (m_state == State::CONNECTED)
    {
        // std::shared_ptr<char[]> buffer = std::shared_ptr<char[]>(new char[UDP::MTU]); // store it as member
        auto pkt = std::make_shared<media_packet_ptr::element_type>();
        pkt->data = new uint8_t[UDP::MTU - sizeof(pkt->header)];
        m_udpSocket->async_receive_from(boost::asio::buffer(pkt.get(), UDP::MTU), m_serverUdpEndpoint, 
            [this, that = shared_from_this(), pkt](const boost::system::error_code& ec, std::size_t bytesTransferred){
                if (!ec)
                {
                    {
                        const std::lock_guard<std::mutex> lock(m_queueMutex);
                        m_writeQueue.push(pkt);
                    }
                    LOG_EX_INFO("Receive media packet: size(bytes) = " + std::to_string(pkt->header.size));
                    ReceiveData();
                } else {
                    LOG_EX_WARN("Unable to receive media packet: " + ec.message());
                }
            });
    }
}
