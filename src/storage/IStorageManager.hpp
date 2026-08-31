#pragma once

#include "configuration/DashcamConfig.hpp"

#include <filesystem>
#include <string>

namespace bike_dashcam::storage {

class IStorageManager {
public:
    virtual ~IStorageManager() = default;

    virtual bool initialize(const configuration::DashcamConfig& config, std::string& error_message) = 0;
    virtual bool isReady() const = 0;
    virtual std::filesystem::path recordingsDirectory() const = 0;
    virtual std::filesystem::path logsDirectory() const = 0;
};

}  // namespace bike_dashcam::storage
