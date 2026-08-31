#pragma once

#include "configuration/DashcamConfig.hpp"

#include <filesystem>
#include <string>
#include <vector>

namespace bike_dashcam::configuration {

class ConfigManager {
public:
    bool loadFromFile(const std::filesystem::path& file_path);

    const DashcamConfig& config() const;
    const std::filesystem::path& configPath() const;
    const std::vector<std::string>& errors() const;

private:
    void assignValue(
        const std::string& section,
        const std::string& key,
        const std::string& value,
        std::size_t line_number);

    static std::string normalizeKey(const std::string& value);

    DashcamConfig config_{};
    std::filesystem::path config_path_;
    std::vector<std::string> errors_;
};

}  // namespace bike_dashcam::configuration
