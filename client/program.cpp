#include "program.h"
#include "streamer.h"
#include "receiver.h"
#include <boost/program_options.hpp>
#include <iostream>
#include <string>
#include <cstdint>
#include <memory>

using namespace boost::program_options;

int main(int argc, char* argv[]) {
    std::string serverTcpIp = "192.11.0.3";
    std::uint16_t serverTcpPort = 35005;
    std::string serverUdpIp = "192.11.0.3";
    std::uint16_t serverUdpPort = 35006;
    std::string streamerUdpIp = "192.11.0.3";
    std::uint16_t streamerUdpPort = 35007;
    std::string receiverUdpIp = "192.11.0.3";
    std::uint16_t receiverUdpPort = 35008;

    std::uint32_t streamId;
    int width = 1280, height = 720;
    int bitrate = 4'000'000, gopSize = 10;
    bool saveRawStreams = false;
    std::string rawStreamsDir = "/tmp";

    options_description desc("Allowed options");
    desc.add_options()
        ("help,h", "Prints this help")
        ("streamer", "Create sreamer")
        ("receiver", "Create receiver")
        ("stream-id", value<std::uint32_t>()->required(), "Stream id")
        ("bitrate", value<int>()->default_value(bitrate)->required(), "Bitrate")
        ("gop-size", value<int>()->default_value(gopSize)->required(), "Gop size")
        ("server-tcp-ip", value<std::string>()->default_value(serverTcpIp)->required(), "Server bind TCP IP")
        ("server-tcp-port", value<std::uint16_t>()->default_value(serverTcpPort)->required(), "Server bind TCP port")
        ("server-udp-ip", value<std::string>()->default_value(serverUdpIp)->required(), "Server bind UDP IP")
        ("server-udp-port", value<std::uint16_t>()->default_value(serverUdpPort)->required(), "Server bind UDP port")
        ("streamer-udp-ip", value<std::string>()->default_value(streamerUdpIp)->required(), "Streamer bind UDP IP")
        ("streamer-udp-port", value<std::uint16_t>()->default_value(streamerUdpPort)->required(), "Streamer bind UDP port")
        ("receiver-udp-ip", value<std::string>()->default_value(receiverUdpIp)->required(), "Receiver bind UDP IP")
        ("receiver-udp-port", value<std::uint16_t>()->default_value(receiverUdpPort)->required(), "Receiver bind UDP port")
    ;

    try {
        variables_map vm;
        store(parse_command_line(argc, argv, desc), vm);
        notify(vm);

        if (vm.count("help")) {  
            std::cout << desc << "\n";
            return 0;
        }

        if (vm.count("stream-id")) {
            streamId = vm["stream-id"].as<std::uint32_t>();
        }
        if (vm.count("bitrate")) {
            bitrate = vm["bitrate"].as<int>();
        }
        if (vm.count("gop-size")) {
            gopSize = vm["gop-size"].as<int>();
        }

        if (vm.count("server-tcp-ip")) {
            serverTcpIp = vm["server-tcp-ip"].as<std::string>();
        }
        if (vm.count("server-tcp-port")) {
            serverTcpPort = vm["server-tcp-port"].as<std::uint16_t>();
        }
        
        if (vm.count("server-udp-ip")) {
            serverUdpIp = vm["server-udp-ip"].as<std::string>();
        }
        if (vm.count("server-udp-port")) {
            serverUdpPort = vm["server-udp-port"].as<std::uint16_t>();
        }

        if (vm.count("streamer-udp-ip")) {
            streamerUdpIp = vm["streamer-udp-ip"].as<std::string>();
        }
        if (vm.count("streamer-udp-port")) {
            streamerUdpPort = vm["streamer-udp-port"].as<std::uint16_t>();
        }

        if (vm.count("receiver-udp-ip")) {
            receiverUdpIp = vm["receiver-udp-ip"].as<std::string>();
        }
        if (vm.count("receiver-udp-port")) {
            receiverUdpPort = vm["receiver-udp-port"].as<std::uint16_t>();
        }

        if (vm.count("streamer")) {  
            auto streamer = std::make_shared<Streamer>();
            streamer->SetServerTcpIp(serverTcpIp);
            streamer->SetServerTcpPort(serverTcpPort);
            streamer->SetServerUdpIp(serverUdpIp);
            streamer->SetServerUdpPort(serverUdpPort);
            streamer->SetLocalUdpIp(streamerUdpIp);
            streamer->SetLocalUdpPort(streamerUdpPort);

            streamer->SetStreamId(streamId);
            streamer->SetWidth(width);
            streamer->SetHeight(height);
            streamer->SetBitrate(bitrate);
            streamer->SetGopSize(gopSize);

            streamer->StartAsync();
            streamer->HandleEvents();
            streamer->Destroy();
        } else if (vm.count("receiver")) {
            auto receiver = std::make_shared<Receiver>();
            receiver->SetServerTcpIp(serverTcpIp);
            receiver->SetServerTcpPort(serverTcpPort);
            receiver->SetServerUdpIp(serverUdpIp);
            receiver->SetServerUdpPort(serverUdpPort);
            receiver->SetLocalUdpIp(receiverUdpIp);
            receiver->SetLocalUdpPort(receiverUdpPort);

            receiver->SetStreamId(streamId);
            receiver->SetWidth(width);
            receiver->SetHeight(height);
            receiver->SetBitrate(bitrate);
            receiver->SetGopSize(gopSize);

            receiver->Start();
            receiver->HandleEvents();
            receiver->Destroy();
        } else {
            std::cerr << "Neither receiver nor streamer were selected" << "\n";
            return 1;
        }
    } catch(const std::exception& e) {
        std::cerr << e.what() << "\n";
        return 1;
    }

    return 0;
}
