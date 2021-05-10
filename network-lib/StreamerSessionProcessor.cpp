#include "StreamerSessionProcessor.h"
#include "NetPacket.h"
#include "NetworkUtils.h"
#include "Logger.h"

StreamerSessionProcessor::StreamerSessionProcessor(boost::asio::io_context& ioContext, const std::uint32_t sessionId)
: m_ioContext(ioContext)
, m_tcpSocket(std::make_shared<boost::asio::ip::tcp::socket>(ioContext))
, m_udpSocket(std::make_shared<boost::asio::ip::udp::socket>(ioContext))
, m_sessionId(sessionId)
, m_sessionState(StreamerSessionState::INITIALIZED)
{
}

void StreamerSessionProcessor::Init()
{
    if (this->m_state != State::INITIALIZED)
    {
        CreateStream();
        DataProcessor::Init();
    }
}

void StreamerSessionProcessor::Destroy()
{
    if (this->m_state != State::DESTROYED)
    {
        DestroyStream();
        DataProcessor::Destroy();
        m_sessionState = StreamerSessionState::STOPPED;
    }
}

void StreamerSessionProcessor::Process(const udp_packet_ptr& pkt)
{
    SendData(pkt);
    // DataProcessor::Process(pkt);
}

void StreamerSessionProcessor::CreateStream()
{
    m_tcpSocket->open(boost::asio::ip::tcp::v4());
    m_tcpSocket->async_connect(m_serverTcpEndpoint, [this, that = shared_from_this()](const boost::system::error_code& ec){
        if (!ec) {
            auto pkt = std::make_shared<net_packet_ptr::element_type>();
            pkt->header.type = net_packet_type_t::CREATE;
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
                                    m_sessionState = StreamerSessionState::SESSION_CREATED;
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

void StreamerSessionProcessor::DestroyStream()
{
    m_tcpSocket->open(boost::asio::ip::tcp::v4());
    m_tcpSocket->async_connect(m_serverTcpEndpoint, [this, that = shared_from_this()](const boost::system::error_code& ec){
        if (!ec) {
            auto pkt = std::make_shared<net_packet_ptr::element_type>();
            pkt->header.type = net_packet_type_t::DESTROY;
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
                                    m_sessionState = StreamerSessionState::SESSION_DESTROYED;
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

void StreamerSessionProcessor::SendData(const udp_packet_ptr& pkt)
{
    if (m_sessionState == StreamerSessionState::SESSION_CREATED)
    {
        m_udpSocket->async_send_to(boost::asio::buffer(pkt.get(), sizeof(pkt->header) + pkt->header.size), m_serverUdpEndpoint, 
            [this, that = shared_from_this(), pkt](const boost::system::error_code& ec, std::size_t bytesTransferred){
                if (!ec) {
                    // LOG_EX_INFO("Udp packet was sent");
                } else {
                    LOG_EX_WARN("Unable to send udp packet: " + ec.message());
                }
            });
    }
}
