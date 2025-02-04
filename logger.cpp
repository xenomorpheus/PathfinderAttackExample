#include "logger.hpp"

Logger::Logger(const std::string &filename)
{
    logFile.open(filename, std::ios::app);
    if (!logFile)
    {
        std::cerr << "Failed to open log file!" << std::endl;
    }
}

Logger::~Logger()
{
    if (logFile.is_open())
    {
        logFile.close();
    }
}

void Logger::setLogLevel(LogLevel level)
{
    logLevel = level;
}

void Logger::log(LogLevel level, std::ostringstream &&stream)
{
    if (!logFile)
    {
        std::cerr << "Logging failed: Log file is not open." << std::endl;
        return;
    }

    std::lock_guard<std::mutex> lock(logMutex);

    if (level < logLevel)
    {
        return;
    }

    logFile << "[" << getTimeStamp() << "] [" << logLevelToString(level) << "] "
            << stream.str() << std::endl;
}

std::string Logger::getTimeStamp()
{
    std::time_t now = std::time(nullptr);
    char buf[20];
    std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", std::localtime(&now));
    return buf;
}

std::string Logger::logLevelToString(LogLevel level)
{
    switch (level)
    {
    case LogLevel::TRACE:
        return "TRACE";
    case LogLevel::DEBUG:
        return "DEBUG";
    case LogLevel::INFO:
        return "INFO";
    case LogLevel::WARNING:
        return "WARNING";
    case LogLevel::ERROR:
        return "ERROR";
    default:
        return "UNKNOWN";
    }
}
