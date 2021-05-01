#pragma once

#include <string>

class Logger
{
public:
    enum class LogLevel
    {
        Info = 0,
        Warning = 1,
        Error = 2
    };
public:
    Logger(const char* methodName, LogLevel logLevel = LogLevel::Info);
    void operator()(const std::string& message);
    void operator()(const char* format, ...);

private:
    void Log(const std::string& message);

private:
    std::string m_buffer;
    const int m_maxBufferLen;
    const char* m_methodName;
    LogLevel m_logLevel;
};

#ifdef _WIN32
    #define CURRENT_FUNCTION __FUNCTION__
#elif defined __linux__ 
    #define CURRENT_FUNCTION __PRETTY_FUNCTION__
#endif

#define LOG_EX Logger(CURRENT_FUNCTION)

#define LOG_EX_INFO Logger(nullptr, Logger::LogLevel::Info)
#define LOG_EX_WARN Logger(nullptr, Logger::LogLevel::Warning)
#define LOG_EX_ERROR Logger(nullptr, Logger::LogLevel::Error)

#define LOG_EX_INFO_WITH_CONTEXT Logger(CURRENT_FUNCTION, Logger::LogLevel::Info)
#define LOG_EX_WARN_WITH_CONTEXT Logger(CURRENT_FUNCTION, Logger::LogLevel::Warning)
#define LOG_EX_ERROR_WITH_CONTEXT Logger(CURRENT_FUNCTION, Logger::LogLevel::Error)
