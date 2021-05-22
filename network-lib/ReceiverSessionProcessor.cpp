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
, m_sessionState(ReceiverSessionState::INITIALIZED)
{
}

void ReceiverSessionProcessor::Init()
{
    m_serverTcpEndpoint = boost::asio::ip::tcp::endpoint(boost::asio::ip::address_v4::from_string(m_serverIp), m_serverTcpPort);
    m_localUdpEndpoint = boost::asio::ip::udp::endpoint(boost::asio::ip::address_v4::from_string(m_localIp), m_localUdpPort);

    ConnectToStream();
    DataProcessor::Init();
}

void ReceiverSessionProcessor::Destroy()
{
    DisconnectFromStream();
    DataProcessor::Destroy();
    m_sessionState = ReceiverSessionState::STOPPED;
}

void ReceiverSessionProcessor::Play()
{
    ReceiveData();
}

void ReceiverSessionProcessor::ConnectToStream()
{
    boost::system::error_code ec;
    m_tcpSocket->open(boost::asio::ip::tcp::v4(), ec);
    if (!ec) {
        m_tcpSocket->async_connect(m_serverTcpEndpoint, [this, that = shared_from_this()](const boost::system::error_code& ec){
            if (!ec) {
                LOG_EX_INFO("Connected to server to connect");
                auto pkt = std::make_shared<net_packet_ptr::element_type>();
                pkt->header.type = net_packet_type_t::CONNECT;
                pkt->header.id = this->m_sessionId;
                m_localIp = m_tcpSocket->local_endpoint().address().to_string();
                m_localUdpPort = m_tcpSocket->local_endpoint().port();
                m_localUdpEndpoint = boost::asio::ip::udp::endpoint(boost::asio::ip::address_v4::from_string(m_localIp), m_localUdpPort);
                pkt->data = NetworkUtils::EncodeUdpAddress(m_localUdpEndpoint);
                pkt->header.size = pkt->data.size();
                boost::asio::async_write(*m_tcpSocket, boost::asio::buffer(&pkt->header, sizeof(pkt->header)), 
                    [this, that, pkt](const boost::system::error_code& ec, std::size_t bytesTransferred){
                        if (!ec) {
                            boost::asio::async_write(*m_tcpSocket, boost::asio::buffer(pkt->data.data(), pkt->data.size()), 
                                [this, that, pkt](const boost::system::error_code& ec, std::size_t bytesTransferred){
                                    if (!ec) {
                                        std::shared_ptr<char[]> roomUdpPortBuffer = std::shared_ptr<char[]>(new char[sizeof(std::uint16_t)]);                                      
                                        boost::asio::async_read(*m_tcpSocket, boost::asio::buffer(&roomUdpPortBuffer[0], sizeof(std::uint16_t)), 
                                            [this, that, pkt, roomUdpPortBuffer](const boost::system::error_code& ec, std::size_t bytesTransferred) {
                                                boost::system::error_code dummy;
                                                m_tcpSocket->shutdown(boost::asio::socket_base::shutdown_both, dummy);
                                                m_tcpSocket->close(dummy);
                                                if (!ec) {
                                                    std::uint16_t roomPort = *(std::uint16_t *)&roomUdpPortBuffer[0];
                                                    m_serverUdpEndpoint = boost::asio::ip::udp::endpoint(boost::asio::ip::address_v4::from_string(m_localIp), roomPort);
                                                    m_udpSocket->open(m_localUdpEndpoint.protocol());
                                                    m_udpSocket->bind(m_localUdpEndpoint);
                                                    m_sessionState = ReceiverSessionState::CONNECTED;
                                                    LOG_EX_INFO("Receiver session started on port = %d", roomPort);
                                                } else {
                                                    LOG_EX_WARN("Unable to read room port: " + ec.message());
                                                }
                                            });
                                    } else {
                                        LOG_EX_WARN_WITH_CONTEXT("Unable to write net packet data: " + ec.message());
                                    }
                                });
                        } else {
                            LOG_EX_WARN_WITH_CONTEXT("Unable to write net packet header: " + ec.message());
                        }
                    });
            } else {
                LOG_EX_WARN_WITH_CONTEXT("Unable to connect to server: " + ec.message());
            }
        });
    } else {
        LOG_EX_WARN_WITH_CONTEXT("Unable open socket: %s", ec.message().c_str());
    }
}

void ReceiverSessionProcessor::DisconnectFromStream()
{
    boost::system::error_code ec;
    m_tcpSocket->open(boost::asio::ip::tcp::v4(), ec);
    if (!ec) {
        m_tcpSocket->async_connect(m_serverTcpEndpoint, [this, that = shared_from_this()](const boost::system::error_code& ec){
            if (!ec) {
                LOG_EX_INFO("Connected to server to disconnect");
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
                                        LOG_EX_INFO("Disconected from server");
                                        m_sessionState = ReceiverSessionState::DISCONNECTED;
                                    } else {
                                        LOG_EX_WARN_WITH_CONTEXT("Unable to write net packet data: " + ec.message());
                                    }
                                });
                        } else {
                            LOG_EX_WARN_WITH_CONTEXT("Unable to write net packet header: " + ec.message());
                        }
                    });
            } else {
                LOG_EX_WARN_WITH_CONTEXT("Unable to connect to server: " + ec.message());
            }
        });
    } else {
        LOG_EX_WARN_WITH_CONTEXT("Unable open socket: %s", ec.message().c_str());
    }
}

void ReceiverSessionProcessor::ReceiveData()
{
    if (m_sessionState == ReceiverSessionState::CONNECTED)
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
                    LOG_EX_WARN_WITH_CONTEXT("Unable to receive udp packet: %s", ec.message().c_str());
                }
            });
    }
}
