#pragma once

#include <cstdint>
#include <string>

class AboutProgramCredentials
{
public:
    AboutProgramCredentials() = delete;

public:
    static inline const std::string programName = "streaming-tool";
    static inline const std::string programVersion = "1.0.0.0";
#if defined(__DATE__) && defined(__TIME__)
    static inline const std::string date = __DATE__;
    static inline const std::string time = __TIME__;
#endif
#ifdef  __GNUC__
    static inline const std::string compilerVersion = __VERSION__;
#endif
#ifdef __cplusplus
    static inline const long cppVersion = __cplusplus;
#endif
#ifdef _WIN32
    static inline const std::string os = "Windows";
#elif defined __linux__ 
    static inline const std::string os = "Linux";
#elif defined __MACH__
    static inline const std::string os = "Mach";
#endif
};
