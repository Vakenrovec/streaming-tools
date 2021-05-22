#pragma once

#include <string>
#include <cstdint>
#include <boost/asio/ip/udp.hpp>
#include <boost/asio/ip/tcp.hpp>

class NetworkUtils
{
public:
    NetworkUtils() = delete;

    static std::string EncodeUdpAddress(boost::asio::ip::udp::endpoint& endpoint);
    static std::string EncodeTcpAddress(boost::asio::ip::tcp::endpoint& endpoint);
    static std::string EncodeAddress(std::string& ip, std::uint16_t port);

    static boost::asio::ip::udp::endpoint DecodeUdpAddress(std::string& address);
    static boost::asio::ip::tcp::endpoint DecodeTcpAddress(std::string& address);

    static std::uint16_t FindFreeUdpPortFrom(const std::uint16_t startPort, const std::uint16_t endPort);
};
