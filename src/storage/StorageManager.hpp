#pragma once

#include "storage/IStorageManager.hpp"

namespace bike_dashcam::storage {

class StorageManager : public IStorageManager {
public:
    bool initialize(const configuration::DashcamConfig& config, std::string& error_message) override;
    bool isReady() const override;
    std::filesystem::path recordingsDirectory() const override;
    std::filesystem::path logsDirectory() const override;

private:
    std::filesystem::path resolvePath(const std::filesystem::path& configured_path) const;

    std::filesystem::path recordings_directory_;
    std::filesystem::path logs_directory_;
    bool ready_{false};
};

}  // namespace bike_dashcam::storage
