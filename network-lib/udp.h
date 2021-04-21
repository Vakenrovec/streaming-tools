#pragma once

class UDP
{
public:
    static constexpr int MTU = 1500;
    static constexpr int IPv4HeaderSize = 20;
    static constexpr int UDPHeaderSize = 8;
    static constexpr int MaxUdpPacketSize = UDP::MTU - UDP::IPv4HeaderSize - UDP::UDPHeaderSize;
};
