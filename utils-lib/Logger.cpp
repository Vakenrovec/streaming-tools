#include "Logger.h"

#include <iostream>
#include <stdarg.h>

Logger::Logger(const char* methodName, LogLevel logLevel)
: m_methodName(methodName)
, m_logLevel(logLevel)
, m_maxBufferLen(1024)
{
    m_buffer.resize(m_maxBufferLen);
}

void Logger::operator()(const std::string& message)
{
    Log(message);
}

void Logger::operator()(const char* format, ...)
{
    va_list args;
    va_start(args, format);
    vsnprintf(m_buffer.data(), m_maxBufferLen, format, args);
    Log(m_buffer);
    va_end(args);
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
    std::string log;
    if (m_methodName)
    {
        log += m_methodName;
        log +=  + ": ";
    }
    log += logLevelName + ": " + message + "\n";
    std::cout << log << std::endl;
}
