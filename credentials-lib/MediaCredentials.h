#pragma once

#include <cstdint>
#include <string>

class MediaCredentials
{
public:
    MediaCredentials() = delete;

public:
    static inline const std::uint32_t streamId = 777;

    static inline const int width = 1280;
    static inline const int height = 720;

    static inline const int bitrate = 4'000'000;
    static inline const int gopSize = 10;
    static inline const int framesDelay = 5;

    static inline const bool saveRawStream = false;
    static inline const std::string rawStreamDir = "/tmp/streams";
    static inline const std::string rawStreamFilename = "stream.raw";
    
    static inline const bool disableAudio = false;
    static inline const bool disableVideo = false;

    static inline const std::string imagePath = "/media/sf_streaming-tools/Streaming-tool.png";
};
