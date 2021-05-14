#pragma once

#include <cstdint>
#include <string>

class Credentials
{
public:
    Credentials() = delete;

    static inline const std::string serverPrivateIp = "192.11.0.3";
    static inline const std::string serverPublicIp = "192.12.0.3";
    static inline const std::uint16_t serverTcpPort = 35005;
    static inline const std::uint16_t serverUdpPort = 35006;

    static inline const std::string streamerPrivateIp = "192.11.0.3";
    static inline const std::string streamerPublicIp = "192.12.0.3";
    static inline const std::uint16_t streamerUdpPort = 35007;
    
    static inline const std::string receiverPrivateIp = "192.11.0.3";
    static inline const std::string receiverUdpPublicIp = "192.12.0.3";
    static inline const std::uint16_t receiverUdpPort = 35008;
};
