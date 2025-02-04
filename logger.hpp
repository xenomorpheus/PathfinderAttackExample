#pragma once

#include <iostream>
#include <fstream>
#include <sstream>
#include <mutex>
#include <ctime>

enum class LogLevel
{
    TRACE,
    DEBUG,
    INFO,
    WARNING,
    ERROR
};

class Logger
{
private:
    LogLevel logLevel = LogLevel::INFO;
    std::ofstream logFile;
    std::mutex logMutex;

    std::string getTimeStamp();
    std::string logLevelToString(LogLevel level);

public:
    Logger(const std::string &filename);
    ~Logger();

    void setLogLevel(LogLevel level);

    void log(LogLevel level, std::ostringstream &&stream);
};
