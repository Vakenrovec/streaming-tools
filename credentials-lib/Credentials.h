#pragma once

#include <cstdint>
#include <string>

class Credentials
{
public:
    Credentials() = delete;

private:
    static inline const std::string m_localPrivateIp = "192.11.0.5";
    
public:
    static inline const std::string serverIp = m_localPrivateIp;
    static inline const std::uint16_t serverTcpPort = 35000;
    static inline const std::uint16_t serverUdpPort = 35001;

    static inline const std::string streamerIp = m_localPrivateIp;
    static inline const std::uint16_t streamerUdpPort = 36000;
    
    static inline const std::string receiverIp = m_localPrivateIp;
    static inline const std::uint16_t receiverUdpPort = 37000;
};
