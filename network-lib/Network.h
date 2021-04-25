#pragma once

namespace Network
{
    constexpr inline int MTU = 1500;
    constexpr inline int IPv4HeaderSize = 20;
    constexpr inline int UDPHeaderSize = 8;
    constexpr inline int MaxUdpPacketSize = MTU - IPv4HeaderSize - UDPHeaderSize;
};
