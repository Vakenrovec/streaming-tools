#include "ReceiverSessionProcessor.h"
#include "NetPacket.h"
#include "Logger.h"
#include "Network.h"
#include "NetworkUtils.h"

ReceiverSessionProcessor::ReceiverSessionProcessor(boost::asio::io_context& ioContext, const std::uint32_t sessionId)
: m_ioContext(ioContext)
, m_tcpSocket(std::make_shared<boost::asio::ip::tcp::socket>(ioContext))
, m_udpSocket(std::make_shared<boost::asio::ip::udp::socket>(ioContext))
, m_sessionId(sessionId)
, m_state(State::INITIALIZED)
{
}

void ReceiverSessionProcessor::Init()
{
    ConnectToStream();
    DataProcessor::Init();
}

void ReceiverSessionProcessor::Destroy()
{
    DisconnectFromStream();
    DataProcessor::Destroy();
    m_state = State::STOPPED;
}

void ReceiverSessionProcessor::Play()
{
    ReceiveData();
}

void ReceiverSessionProcessor::ConnectToStream()
{
    m_tcpSocket->open(boost::asio::ip::tcp::v4());
    m_tcpSocket->async_connect(m_serverTcpEndpoint, [this, that = shared_from_this()](const boost::system::error_code& ec){
        if (!ec) {
            auto pkt = std::make_shared<net_packet_ptr::element_type>();
            pkt->header.type = net_packet_type_t::CONNECT;
            pkt->header.id = this->m_sessionId;
            pkt->data = NetworkUtils::EncodeUdpAddress(m_localUdpEndpoint);
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

void ReceiverSessionProcessor::DisconnectFromStream()
{
    m_tcpSocket->open(boost::asio::ip::tcp::v4());
    m_tcpSocket->async_connect(m_serverTcpEndpoint, [this, that = shared_from_this()](const boost::system::error_code& ec){
        if (!ec) {
            auto pkt = std::make_shared<net_packet_ptr::element_type>();
            pkt->header.type = net_packet_type_t::DISCONNECT;
            pkt->header.id = this->m_sessionId;
            pkt->data = NetworkUtils::EncodeUdpAddress(m_localUdpEndpoint);
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
                                    if (m_udpSocket->is_open())
                                    {
                                        m_udpSocket->shutdown(boost::asio::socket_base::shutdown_both, ec);
                                        m_udpSocket->close();
                                    }
                                    m_state = State::DISCONNECTED;
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

void ReceiverSessionProcessor::ReceiveData()
{
    if (m_state == State::CONNECTED)
    {
        auto pkt = std::make_shared<udp_packet_t>();
        m_udpSocket->async_receive_from(boost::asio::buffer(pkt.get(), Network::MaxUdpPacketSize), m_serverUdpEndpoint, 
            [this, that = shared_from_this(), pkt](const boost::system::error_code& ec, std::size_t bytesTransferred){
                if (!ec)
                {
                    // LOG_EX_INFO("Received udp packet: size(bytes) = " + std::to_string(pkt->header.size));
                    DataProcessor::Process(pkt);
                    ReceiveData();
                } else {
                    LOG_EX_WARN("Unable to receive udp packet: " + ec.message());
                }
            });
    }
}
