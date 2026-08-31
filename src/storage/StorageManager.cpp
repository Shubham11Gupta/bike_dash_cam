#include "storage/StorageManager.hpp"

#include <exception>

namespace bike_dashcam::storage {

bool StorageManager::initialize(const configuration::DashcamConfig& config, std::string& error_message) {
    try {
        recordings_directory_ = resolvePath(config.application.recordings_directory);
        logs_directory_ = resolvePath(config.application.logs_directory);

        std::filesystem::create_directories(recordings_directory_);
        std::filesystem::create_directories(logs_directory_);

        ready_ = true;
        return true;
    } catch (const std::exception& exception) {
        ready_ = false;
        error_message = exception.what();
        return false;
    }
}

bool StorageManager::isReady() const {
    return ready_;
}

std::filesystem::path StorageManager::recordingsDirectory() const {
    return recordings_directory_;
}

std::filesystem::path StorageManager::logsDirectory() const {
    return logs_directory_;
}

std::filesystem::path StorageManager::resolvePath(const std::filesystem::path& configured_path) const {
    if (configured_path.empty()) {
        return std::filesystem::current_path();
    }

    if (configured_path.is_absolute()) {
        return configured_path;
    }

    return std::filesystem::current_path() / configured_path;
}

}  // namespace bike_dashcam::storage
