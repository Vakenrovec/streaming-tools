#pragma once

#include <cstdint>
#include <string>

class AboutProgramCredentials
{
public:
    AboutProgramCredentials() = delete;

public:
    static inline const std::string date = __DATE__;
    static inline const std::string version = "1.0.0.0";
    static inline const std::string programName = "streaming-tools";
#ifdef _WIN32
    static inline const std::string os = "Windows";
#elif defined __linux__ 
    static inline const std::string os = "Linux";
#elif defined __MACH__
    static inline const std::string os = "Mach";
#endif
};
