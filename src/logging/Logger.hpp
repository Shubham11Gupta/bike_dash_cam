#pragma once

#include <string>

enum class LogLevel
{
    DEBUG,
    INFO,
    WARN,
    ERROR
};

class Logger
{
public:
    static bool initialize(
        const std::string& log_level,
        const std::string& log_file = "./logs/dashcam.log");

    static void debug(const std::string& message);
    static void info(const std::string& message);
    static void warn(const std::string& message);
    static void error(const std::string& message);

private:
    static void log(
        LogLevel level,
        const std::string& message);

    static LogLevel parseLogLevel(
        const std::string& log_level);

    static std::string levelToString(
        LogLevel level);

    static bool shouldLog(
        LogLevel level);

    static LogLevel minimum_level_;
    static std::string log_file_;
    static bool initialized_;
};