#include "logging/Logger.hpp"

#include <cctype>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <sstream>

namespace bike_dashcam::logging {

std::string_view toString(const LogLevel level) {
    switch (level) {
        case LogLevel::Debug:
            return "DEBUG";
        case LogLevel::Info:
            return "INFO";
        case LogLevel::Warning:
            return "WARN";
        case LogLevel::Error:
            return "ERROR";
    }

    return "UNKNOWN";
}

std::optional<LogLevel> parseLogLevel(std::string_view value) {
    std::string normalized;
    normalized.reserve(value.size());

    for (const char character : value) {
        normalized.push_back(static_cast<char>(std::toupper(static_cast<unsigned char>(character))));
    }

    if (normalized == "DEBUG") {
        return LogLevel::Debug;
    }
    if (normalized == "INFO") {
        return LogLevel::Info;
    }
    if (normalized == "WARN" || normalized == "WARNING") {
        return LogLevel::Warning;
    }
    if (normalized == "ERROR") {
        return LogLevel::Error;
    }

    return std::nullopt;
}

Logger::Logger(const LogLevel minimum_level) : minimum_level_(minimum_level) {
}

void Logger::setMinimumLevel(const LogLevel minimum_level) {
    std::lock_guard<std::mutex> lock(mutex_);
    minimum_level_ = minimum_level;
}

LogLevel Logger::minimumLevel() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return minimum_level_;
}

void Logger::log(const LogLevel level, const std::string_view component, const std::string_view message) {
    if (!shouldLog(level)) {
        return;
    }

    std::lock_guard<std::mutex> lock(mutex_);
    std::ostream& output = (level == LogLevel::Error) ? std::cerr : std::clog;
    output << '[' << timestampNow() << "] [" << toString(level) << "] [" << component << "] " << message << '\n';
}

bool Logger::shouldLog(const LogLevel level) const {
    return static_cast<int>(level) >= static_cast<int>(minimumLevel());
}

std::string Logger::timestampNow() {
    const auto now = std::chrono::system_clock::now();
    const std::time_t current_time = std::chrono::system_clock::to_time_t(now);

    std::tm local_time{};
#if defined(_WIN32)
    localtime_s(&local_time, &current_time);
#else
    localtime_r(&current_time, &local_time);
#endif

    std::ostringstream stream;
    stream << std::put_time(&local_time, "%Y-%m-%d %H:%M:%S");
    return stream.str();
}

}  // namespace bike_dashcam::logging
