#include "NetworkUtils.h"

std::string NetworkUtils::EncodeUdpAddress(boost::asio::ip::udp::endpoint& endpoint)
{
    return std::string(endpoint.address().to_string() + ":" +  std::to_string(endpoint.port()));
}

std::string NetworkUtils::EncodeTcpAddress(boost::asio::ip::tcp::endpoint& endpoint)
{
    return std::string(endpoint.address().to_string() + ":" +  std::to_string(endpoint.port()));
}

std::string NetworkUtils::EncodeAddress(std::string& ip, std::uint16_t port)
{
    return std::string(ip + ":" +  std::to_string(port));
}

boost::asio::ip::udp::endpoint NetworkUtils::DecodeUdpAddress(std::string& address)
{
    std::string ip = address.substr(0, address.find(':', 0));
    std::string port = address.substr(address.find(':', 0) + 1);
    return boost::asio::ip::udp::endpoint(boost::asio::ip::address_v4::from_string(ip), std::atoi(port.c_str()));
}

boost::asio::ip::tcp::endpoint NetworkUtils::DecodeTcpAddress(std::string& address)
{
    std::string ip = address.substr(0, address.find(':', 0));
    std::string port = address.substr(address.find(':', 0) + 1);
    return boost::asio::ip::tcp::endpoint(boost::asio::ip::address_v4::from_string(ip), std::atoi(port.c_str()));
}

std::uint16_t NetworkUtils::FindFreeUdpPortFrom(const std::uint16_t startPort, const std::uint16_t endPort)
{
    // boost::asio::ip::udp::endpoint
    // boost::asio::ip::udp::socket
    std::uint16_t freePort = startPort;
    freePort++;
    return freePort;
}
