#include "Logger.h"

#include <iostream>

Logger::Logger(const char* methodName, LogLevel logLevel)
: m_methodName(methodName)
, m_logLevel(logLevel)
{
}

void Logger::operator()(const std::string& message)
{
    Log(message);
}

void Logger::Log(const std::string& message)
{
    std::string logLevelName;
    switch (m_logLevel)
    {
    case LogLevel::Info:
        logLevelName = "Info";
        break;
    case LogLevel::Warning:
        logLevelName = "Warning";
        break;
    case LogLevel::Error:
        logLevelName = "Error";
        break;
    default:
        logLevelName = "Info";
        break;
    }
    std::cout << logLevelName << ": ";
    if (m_methodName)
    {
        std::cout << m_methodName << ": ";
    }
    std::cout << message << std::endl;
}
