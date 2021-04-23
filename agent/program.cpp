#include "program.h"
#include "Agent.h"
#include "Logger.h"
#include "ThreadPoolManager.h"
#include <boost/program_options.hpp>
#include <iostream>

using namespace boost::program_options;

int main(int argc, char* argv[])
{
    std::string tcpIp = "192.11.0.3";
    std::uint16_t tcpPort = 35005;
    std::string udpIp = "192.11.0.3";
    std::uint16_t udpPort = 35006;
    std::uint32_t threadsCount = 2;
    bool saveRawStreams = false;
    std::string rawStreamsDir =" /tmp";

    options_description desc("Allowed options");
    desc.add_options()
        ("help,h", "Prints this help")
        ("local-tcp-ip", value<std::string>()->default_value(tcpIp)->required(), "Local bind TCP IP")
        ("local-tcp-port", value<std::uint16_t>()->default_value(tcpPort)->required(), "Local bind TCP port")
        ("local-udp-ip", value<std::string>()->default_value(udpIp)->required(), "Local bind UDP IP")
        ("local-udp-port", value<std::uint16_t>()->default_value(udpPort)->required(), "Local bind UDP port")
        ("threads-count", value<std::uint16_t>()->default_value(threadsCount)->required(), "Concurent threads count")
        ("save-raw-video-streams", value<bool>()->default_value(saveRawStreams), "Enables/disables raw streams recording")
        ("raw-streams-dir", value<std::string>()->default_value(rawStreamsDir)->required(), "The dir where all raw files will be stored")
    ;

    try {
        variables_map vm;
        store(parse_command_line(argc, argv, desc), vm);
        notify(vm);

        if (vm.count("help")) {  
            std::cout << desc << "\n";
            return 0;
        }

        if (vm.count("local-tcp-ip")) {
            tcpIp = vm["local-tcp-ip"].as<std::string>();
        }

        if (vm.count("local-tcp-port")) {
            tcpPort = vm["local-tcp-port"].as<std::uint16_t>();
        }

        if (vm.count("local-udp-ip")) {
            udpIp = vm["local-udp-ip"].as<std::string>();
        }

        if (vm.count("local-udp-port")) {
            udpPort = vm["local-udp-port"].as<std::uint16_t>();
        }

        if (vm.count("threads-count")) {
            threadsCount = vm["threads-count"].as<std::uint16_t>();
        }

        saveRawStreams = vm.count("save-raw-video-streams") && vm["save-raw-video-streams"].as<bool>();

        if (vm.count("raw-streams-dir")) {
            rawStreamsDir = vm["raw-streams-dir"].as<std::string>();
        }        
    } catch(const std::exception& e) {
        std::cerr << e.what() << "\n";
        return 1;
    }

    ThreadPoolManager::GetInstance()->Start(threadsCount);

    auto agent = std::make_shared<Agent>(ThreadPoolManager::GetInstance()->Get());
    agent->SetTcpEndpoint(tcpIp, tcpPort);
    agent->SetUdpEndpoint(udpIp, udpPort);
    agent->SetRawStreamsDir(rawStreamsDir);
    agent->SetSaveRawStreams(saveRawStreams);
    agent->Start();

    ThreadPoolManager::GetInstance()->Get().run();

    return 0;
}
