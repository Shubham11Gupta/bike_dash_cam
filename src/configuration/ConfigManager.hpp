#pragma once

#include <string>

struct SystemConfig
{
    std::string log_level;
};

struct CameraConfig
{
    bool enabled;
    std::string resolution;
    int fps;
};

struct RecordingConfig
{
    std::string codec;
    int segment_duration;
};

struct StorageConfig
{
    int max_usage_percent;
    std::string recording_path;
};

class ConfigManager
{
public:
    ConfigManager();

    bool load(const std::string& file_path);

    const SystemConfig& getSystemConfig() const;
    const CameraConfig& getFrontCameraConfig() const;
    const CameraConfig& getRearCameraConfig() const;
    const RecordingConfig& getRecordingConfig() const;
    const StorageConfig& getStorageConfig() const;

private:
    SystemConfig system_config_;
    CameraConfig front_camera_config_;
    CameraConfig rear_camera_config_;
    RecordingConfig recording_config_;
    StorageConfig storage_config_;
};