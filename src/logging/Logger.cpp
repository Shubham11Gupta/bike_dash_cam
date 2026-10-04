#include "Logger.hpp"

#include <chrono>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>

LogLevel Logger::minimum_level_ = LogLevel::INFO;
std::string Logger::log_file_ = "./logs/dashcam.log";
bool Logger::initialized_ = false;

bool Logger::initialize(
    const std::string& log_level,
    const std::string& log_file)
{
    minimum_level_ = parseLogLevel(log_level);
    log_file_ = log_file;

    try
    {
        std::filesystem::path log_path(log_file_);

        if (log_path.has_parent_path())
        {
            std::filesystem::create_directories(
                log_path.parent_path());
        }

        std::ofstream test_file(
            log_file_,
            std::ios::app);

        if (!test_file.is_open())
        {
            std::cerr
                << "Failed to open log file: "
                << log_file_
                << std::endl;

            return false;
        }

        test_file.close();

        initialized_ = true;

        return true;
    }
    catch (const std::filesystem::filesystem_error& error)
    {
        std::cerr
            << "Failed to initialize logger: "
            << error.what()
            << std::endl;

        return false;
    }
}

void Logger::debug(const std::string& message)
{
    log(LogLevel::DEBUG, message);
}

void Logger::info(const std::string& message)
{
    log(LogLevel::INFO, message);
}

void Logger::warn(const std::string& message)
{
    log(LogLevel::WARN, message);
}

void Logger::error(const std::string& message)
{
    log(LogLevel::ERROR, message);
}

void Logger::log(
    LogLevel level,
    const std::string& message)
{
    if (!initialized_)
    {
        return;
    }

    if (!shouldLog(level))
    {
        return;
    }

    const auto now =
        std::chrono::system_clock::now();

    const std::time_t current_time =
        std::chrono::system_clock::to_time_t(now);

    std::tm local_time{};

#ifdef _WIN32
    localtime_s(
        &local_time,
        &current_time);
#else
    localtime_r(
        &current_time,
        &local_time);
#endif

    std::ostringstream timestamp;

    timestamp
        << std::put_time(
            &local_time,
            "%Y-%m-%d %H:%M:%S");

    const std::string formatted_message =
        timestamp.str() +
        " " +
        levelToString(level) +
        " " +
        message;

    std::cout
        << formatted_message
        << std::endl;

    std::ofstream log_file(
        log_file_,
        std::ios::app);

    if (log_file.is_open())
    {
        log_file
            << formatted_message
            << std::endl;
    }
}

LogLevel Logger::parseLogLevel(
    const std::string& log_level)
{
    if (log_level == "DEBUG")
    {
        return LogLevel::DEBUG;
    }

    if (log_level == "WARN")
    {
        return LogLevel::WARN;
    }

    if (log_level == "ERROR")
    {
        return LogLevel::ERROR;
    }

    return LogLevel::INFO;
}

std::string Logger::levelToString(
    LogLevel level)
{
    switch (level)
    {
        case LogLevel::DEBUG:
            return "DEBUG";

        case LogLevel::INFO:
            return "INFO";

        case LogLevel::WARN:
            return "WARN";

        case LogLevel::ERROR:
            return "ERROR";

        default:
            return "UNKNOWN";
    }
}

bool Logger::shouldLog(
    LogLevel level)
{
    return static_cast<int>(level) >=
           static_cast<int>(minimum_level_);
}