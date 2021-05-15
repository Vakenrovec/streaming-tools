#include "program.h"
#include "Credentials.h"
#include "client.h"
#include "streamer.h"
#include "receiver.h"
#include "Logger.h"
#include <iostream>
#include <string>
#include <cstdint>
#include <memory>
#include <boost/program_options.hpp>

using namespace boost::program_options;

int main(int argc, char* argv[]) {
    std::string serverIp = Credentials::serverIp;
    std::uint16_t serverTcpPort = Credentials::serverTcpPort;
    std::uint16_t serverUdpPort = Credentials::serverUdpPort;
    std::string localIp = Credentials::streamerIp;
    std::uint16_t localUdpPort = Credentials::streamerUdpPort;

    std::uint32_t streamId = 777;
    int width = 1280, height = 720;
    int bitrate = 4'000'000, gopSize = 10;

    bool saveRawStream = false;
    std::string rawStreamDir = "/tmp/streams";
    std::string rawStreamFilename = "stream.raw";

    bool disableAudio = false;
    bool disableVideo = false;

    options_description desc("Allowed options");
    desc.add_options()
        ("help,h", "Prints this help")
        ("streamer", "Create sreamer")
        ("receiver", "Create receiver")
        ("stream-id", value<std::uint32_t>()->default_value(streamId)->required(), "Stream id")
        ("bitrate", value<int>()->default_value(bitrate)->required(), "Bitrate")
        ("gop-size", value<int>()->default_value(gopSize)->required(), "Gop size")

        ("server-ip", value<std::string>()->default_value(serverIp)->required(), "Server bind IP")
        ("server-tcp-port", value<std::uint16_t>()->default_value(serverTcpPort)->required(), "Server bind TCP port")
        ("server-udp-port", value<std::uint16_t>()->default_value(serverUdpPort)->required(), "Server bind UDP port")
        
        ("local-ip", value<std::string>()->default_value(localIp)->required(), "Local bind IP")
        ("local-udp-port", value<std::uint16_t>()->default_value(localUdpPort)->required(), "Local bind UDP port")
        
        ("disable-audio", value<bool>()->default_value(disableAudio), "Disable audio")
        ("disable-video", value<bool>()->default_value(disableVideo), "Disable video")

        ("save-raw-stream", value<bool>()->default_value(saveRawStream), "Save raw stream")
        ("raw-stream-dir", value<std::string>()->default_value(rawStreamDir), "Raw stream direction")
        ("raw-stream-filename", value<std::string>()->default_value(rawStreamFilename), "Raw stream filename")
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

        if (vm.count("server-ip")) {
            serverIp = vm["server-ip"].as<std::string>();
        }
        if (vm.count("server-tcp-port")) {
            serverTcpPort = vm["server-tcp-port"].as<std::uint16_t>();
        }
        if (vm.count("server-udp-port")) {
            serverUdpPort = vm["server-udp-port"].as<std::uint16_t>();
        }

        if (vm.count("local-ip")) {
            localIp = vm["local-ip"].as<std::string>();
        }
        if (vm.count("local-udp-port")) {
            localUdpPort = vm["local-udp-port"].as<std::uint16_t>();
        }

        if (vm.count("disable-audio")) {
            disableAudio = vm["disable-audio"].as<bool>();
        }
        if (vm.count("disable-video")) {
            disableVideo = vm["disable-video"].as<bool>();
        }

        if (vm.count("save-raw-stream")) {
            saveRawStream = vm["save-raw-stream"].as<bool>();
        }
        if (vm.count("raw-stream-dir")) {
            rawStreamDir = vm["raw-stream-dir"].as<std::string>();
        }
        if (vm.count("raw-stream-filename")) {
            rawStreamFilename = vm["raw-stream-filename"].as<std::string>();
        }
        
        std::shared_ptr<IClient> client = nullptr;
        if (vm.count("streamer")) {  
            client = std::make_shared<Streamer>();
        } else if (vm.count("receiver")) {
            client = std::make_shared<Receiver>();
        } else {
            std::cerr << "Neither receiver nor streamer were selected" << "\n";
            return 1;
        }

        client->SetServerIp(serverIp);
        client->SetServerTcpPort(serverTcpPort);
        client->SetServerUdpPort(serverUdpPort);
        client->SetLocalIp(localIp);
        client->SetLocalUdpPort(localUdpPort);

        client->SetStreamId(streamId);
        client->SetWidth(width);
        client->SetHeight(height);
        client->SetBitrate(bitrate);
        client->SetGopSize(gopSize);

        client->SetDisableAudio(disableAudio);
        client->SetDisableVideo(disableVideo);

        client->SetSaveRawStream(saveRawStream);
        client->SetRawStreamDir(rawStreamDir);
        client->SetRawStreamFilename(rawStreamFilename);

        client->StartAsync();
        client->HandleEvents();
        client->Destroy();        
    } catch (const std::exception& e) {
        std::cerr << e.what() << "\n";
        return 1;
    } catch (...) {
        std::cerr << "Unknown exception\n";
        return 1;
    }

    return 0;
}
