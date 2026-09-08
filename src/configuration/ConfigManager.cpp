#include "ConfigManager.hpp"

#include <iostream>

#include <yaml-cpp/yaml.h>

ConfigManager::ConfigManager()
{
}

bool ConfigManager::load(const std::string& file_path)
{
    try
    {
        YAML::Node config = YAML::LoadFile(file_path);

        // System configuration
        system_config_.log_level =
            config["system"]["log_level"].as<std::string>();

        // Front camera configuration
        front_camera_config_.enabled =
            config["cameras"]["front"]["enabled"].as<bool>();

        front_camera_config_.resolution =
            config["cameras"]["front"]["resolution"].as<std::string>();

        front_camera_config_.fps =
            config["cameras"]["front"]["fps"].as<int>();

        // Rear camera configuration
        rear_camera_config_.enabled =
            config["cameras"]["rear"]["enabled"].as<bool>();

        rear_camera_config_.resolution =
            config["cameras"]["rear"]["resolution"].as<std::string>();

        rear_camera_config_.fps =
            config["cameras"]["rear"]["fps"].as<int>();

        // Recording configuration
        recording_config_.codec =
            config["recording"]["codec"].as<std::string>();

        recording_config_.segment_duration =
            config["recording"]["segment_duration"].as<int>();

        // Storage configuration
        storage_config_.max_usage_percent =
            config["storage"]["max_usage_percent"].as<int>();

        storage_config_.recording_path =
            config["storage"]["recording_path"].as<std::string>();

        std::cout << "Configuration loaded successfully."
                  << std::endl;

        return true;
    }
    catch (const YAML::Exception& exception)
    {
        std::cerr << "Failed to load configuration: "
                  << exception.what()
                  << std::endl;

        return false;
    }
}

const SystemConfig& ConfigManager::getSystemConfig() const
{
    return system_config_;
}

const CameraConfig& ConfigManager::getFrontCameraConfig() const
{
    return front_camera_config_;
}

const CameraConfig& ConfigManager::getRearCameraConfig() const
{
    return rear_camera_config_;
}

const RecordingConfig& ConfigManager::getRecordingConfig() const
{
    return recording_config_;
}

const StorageConfig& ConfigManager::getStorageConfig() const
{
    return storage_config_;
}