#pragma once

#include <mutex>
#include <optional>
#include <string>
#include <string_view>

namespace bike_dashcam::logging {

enum class LogLevel {
    Debug = 0,
    Info = 1,
    Warning = 2,
    Error = 3
};

std::string_view toString(LogLevel level);
std::optional<LogLevel> parseLogLevel(std::string_view value);

class Logger {
public:
    explicit Logger(LogLevel minimum_level = LogLevel::Info);

    void setMinimumLevel(LogLevel minimum_level);
    LogLevel minimumLevel() const;
    void log(LogLevel level, std::string_view component, std::string_view message);

private:
    bool shouldLog(LogLevel level) const;
    static std::string timestampNow();

    mutable std::mutex mutex_;
    LogLevel minimum_level_;
};

}  // namespace bike_dashcam::logging
