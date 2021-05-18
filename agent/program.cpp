#include "program.h"
#include "Credentials.h"
#include "Agent.h"
#include "Logger.h"
#include "ThreadPoolManager.h"
#include <boost/program_options.hpp>
#include <iostream>

using namespace boost::program_options;

int main(int argc, char* argv[])
{
    std::string ip = NetworkCredentials::serverIp;
    std::uint16_t tcpPort = NetworkCredentials::serverTcpPort;
    std::uint16_t udpPort = NetworkCredentials::serverUdpPort;
    std::uint32_t threadsCount = 2;

    options_description desc("Allowed options");
    desc.add_options()
        ("help,h", "Prints this help")
        ("local-ip", value<std::string>()->default_value(ip)->required(), "Local bind IP")
        ("local-tcp-port", value<std::uint16_t>()->default_value(tcpPort)->required(), "Local bind TCP port")
        ("local-udp-port", value<std::uint16_t>()->default_value(udpPort)->required(), "Local bind UDP port")
        ("threads-count", value<std::uint16_t>()->default_value(threadsCount)->required(), "Concurent threads count")
    ;

    try {
        variables_map vm;
        store(parse_command_line(argc, argv, desc), vm);
        notify(vm);

        if (vm.count("help")) {  
            std::cout << desc << "\n";
            return 0;
        }

        if (vm.count("local-ip")) {
            ip = vm["local-ip"].as<std::string>();
        }
        if (vm.count("local-tcp-port")) {
            tcpPort = vm["local-tcp-port"].as<std::uint16_t>();
        }
        if (vm.count("local-udp-port")) {
            udpPort = vm["local-udp-port"].as<std::uint16_t>();
        }

        if (vm.count("threads-count")) {
            threadsCount = vm["threads-count"].as<std::uint16_t>();
        }      
    } catch(const std::exception& e) {
        std::cerr << e.what() << "\n";
        return 1;
    }

    ThreadPoolManager::GetInstance()->Start(threadsCount);

    auto agent = std::make_shared<Agent>(ThreadPoolManager::GetInstance()->Get());
    agent->SetIp(ip);
    agent->SetTcpPort(tcpPort);
    agent->SetUdpPort(udpPort);
    agent->Start();

    ThreadPoolManager::GetInstance()->Get().run();

    return 0;
}
