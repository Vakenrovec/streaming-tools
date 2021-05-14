#include "program.h"
#include "client.h"
#include "streamer.h"
#include "receiver.h"
#include "Logger.h"
#include <iostream>
#include <string>
#include <cstdint>
#include <memory>
#include <boost/program_options.hpp>
#include <gtkmm/application.h>

using namespace boost::program_options;

int main(int argc, char* argv[]) {
    std::string serverIp = "192.11.0.3";
    std::uint16_t serverTcpPort = 35005;
    std::uint16_t serverUdpPort = 35006;
    std::string streamerIp = "192.11.0.3";
    std::uint16_t streamerUdpPort = 35007;
    std::string receiverIp = "192.11.0.3";
    std::uint16_t receiverUdpPort = 35008;

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
        ("streamer-ip", value<std::string>()->default_value(streamerIp)->required(), "Streamer bind IP")
        ("streamer-udp-port", value<std::uint16_t>()->default_value(streamerUdpPort)->required(), "Streamer bind UDP port")
        ("receiver-ip", value<std::string>()->default_value(receiverIp)->required(), "Receiver bind IP")
        ("receiver-udp-port", value<std::uint16_t>()->default_value(receiverUdpPort)->required(), "Receiver bind UDP port")

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

        if (vm.count("streamer-ip")) {
            streamerIp = vm["streamer-ip"].as<std::string>();
        }
        if (vm.count("streamer-udp-port")) {
            streamerUdpPort = vm["streamer-udp-port"].as<std::uint16_t>();
        }

        if (vm.count("receiver-ip")) {
            receiverIp = vm["receiver-ip"].as<std::string>();
        }
        if (vm.count("receiver-udp-port")) {
            receiverUdpPort = vm["receiver-udp-port"].as<std::uint16_t>();
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
        
        auto app = Gtk::Application::create("org.gtkmm.main");
        // MainWindow mainWindow;
        // int exitCode = app->run(mainWindow, argc, argv);
        
        if (vm.count("streamer")) {  
            auto streamer = std::make_shared<Streamer>();
            streamer->SetServerIp(serverIp);
            streamer->SetServerTcpPort(serverTcpPort);
            streamer->SetServerUdpPort(serverUdpPort);
            streamer->SetLocalIp(streamerIp);
            streamer->SetLocalUdpPort(streamerUdpPort);

            streamer->SetStreamId(streamId);
            streamer->SetWidth(width);
            streamer->SetHeight(height);
            streamer->SetBitrate(bitrate);
            streamer->SetGopSize(gopSize);

            streamer->SetDisableAudio(disableAudio);
            streamer->SetDisableVideo(disableVideo);

            streamer->SetSaveRawStream(saveRawStream);
            streamer->SetRawStreamDir(rawStreamDir);
            streamer->SetRawStreamFilename(rawStreamFilename);

            streamer->StartAsync();
            streamer->HandleEvents();
            streamer->Destroy();
        } else if (vm.count("receiver")) {
            auto receiver = std::make_shared<Receiver>();
            receiver->SetServerIp(serverIp);
            receiver->SetServerTcpPort(serverTcpPort);
            receiver->SetServerUdpPort(serverUdpPort);
            receiver->SetLocalIp(receiverIp);
            receiver->SetLocalUdpPort(receiverUdpPort);

            receiver->SetStreamId(streamId);
            receiver->SetWidth(width);
            receiver->SetHeight(height);
            receiver->SetBitrate(bitrate);
            receiver->SetGopSize(gopSize);

            receiver->SetDisableAudio(disableAudio);
            receiver->SetDisableVideo(disableVideo);

            receiver->SetSaveRawStream(saveRawStream);
            receiver->SetRawStreamDir(rawStreamDir);
            receiver->SetRawStreamFilename(rawStreamFilename);

            receiver->StartAsync();
            receiver->HandleEvents();
            receiver->Destroy();
        } else {
            std::cerr << "Neither receiver nor streamer were selected" << "\n";
            return 1;
        }
    } catch (const std::exception& e) {
        std::cerr << e.what() << "\n";
        return 1;
    } catch (...) {
        std::cerr << "Unknown exception\n";
        return 1;
    }

    return 0;
}
