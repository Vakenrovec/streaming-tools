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
    static inline const std::uint16_t serverTcpPort = 35005;
    static inline const std::uint16_t serverUdpPort = 35006;

    static inline const std::string streamerIp = m_localPrivateIp;
    static inline const std::uint16_t streamerUdpPort = 35007;
    
    static inline const std::string receiverIp = m_localPrivateIp;
    static inline const std::uint16_t receiverUdpPort = 35008;
};
