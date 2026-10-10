#include "ConfigManager.hpp"

#include "../logging/Logger.hpp"

#include <yaml-cpp/yaml.h>

ConfigManager::ConfigManager()
{
    system_config_.log_level = "INFO";

    front_camera_config_.enabled = true;
    front_camera_config_.resolution = "1920x1080";
    front_camera_config_.fps = 30;

    rear_camera_config_.enabled = true;
    rear_camera_config_.resolution = "1920x1080";
    rear_camera_config_.fps = 30;

    recording_config_.codec = "h264";
    recording_config_.segment_duration = 300;

    storage_config_.max_usage_percent = 90;
    storage_config_.recording_path = "./recordings";
}

bool ConfigManager::load(const std::string& file_path)
{
    try
    {
        YAML::Node config = YAML::LoadFile(file_path);

        // System configuration
        if (config["system"] && config["system"]["log_level"])
        {
            system_config_.log_level =
                config["system"]["log_level"].as<std::string>();
        }

        // Camera configuration
        if (config["cameras"])
        {
            if (config["cameras"]["front"])
            {
                const auto& front = config["cameras"]["front"];

                if (front["enabled"])
                {
                    front_camera_config_.enabled =
                        front["enabled"].as<bool>();
                }

                if (front["resolution"])
                {
                    front_camera_config_.resolution =
                        front["resolution"].as<std::string>();
                }

                if (front["fps"])
                {
                    front_camera_config_.fps =
                        front["fps"].as<int>();
                }
            }

            if (config["cameras"]["rear"])
            {
                const auto& rear = config["cameras"]["rear"];

                if (rear["enabled"])
                {
                    rear_camera_config_.enabled =
                        rear["enabled"].as<bool>();
                }

                if (rear["resolution"])
                {
                    rear_camera_config_.resolution =
                        rear["resolution"].as<std::string>();
                }

                if (rear["fps"])
                {
                    rear_camera_config_.fps =
                        rear["fps"].as<int>();
                }
            }
        }

        // Recording configuration
        if (config["recording"])
        {
            if (config["recording"]["codec"])
            {
                recording_config_.codec =
                    config["recording"]["codec"].as<std::string>();
            }

            if (config["recording"]["segment_duration"])
            {
                recording_config_.segment_duration =
                    config["recording"]["segment_duration"].as<int>();
            }
        }

        // Storage configuration
        if (config["storage"])
        {
            if (config["storage"]["max_usage_percent"])
            {
                storage_config_.max_usage_percent =
                    config["storage"]["max_usage_percent"].as<int>();
            }

            if (config["storage"]["recording_path"])
            {
                storage_config_.recording_path =
                    config["storage"]["recording_path"].as<std::string>();
            }
        }

        if (!validate())
        {
            Logger::error(
                "Configuration validation failed."
            );

            return false;
        }

        Logger::info(
            "Configuration loaded and validated successfully."
        );

        return true;
    }
    catch (const YAML::Exception& exception)
    {
        Logger::error(
            "Failed to load configuration: " +
            std::string(exception.what())
        );

        return false;
    }
}

bool ConfigManager::validate() const
{
    // Front camera
    if (front_camera_config_.enabled)
    {
        if (front_camera_config_.resolution.empty())
        {
            Logger::error(
                "Invalid front camera resolution."
            );

            return false;
        }

        if (front_camera_config_.fps <= 0)
        {
            Logger::error(
                "Invalid front camera FPS."
            );

            return false;
        }
    }

    // Rear camera
    if (rear_camera_config_.enabled)
    {
        if (rear_camera_config_.resolution.empty())
        {
            Logger::error(
                "Invalid rear camera resolution."
            );

            return false;
        }

        if (rear_camera_config_.fps <= 0)
        {
            Logger::error(
                "Invalid rear camera FPS."
            );

            return false;
        }
    }

    // At least one camera must be enabled
    if (!front_camera_config_.enabled &&
        !rear_camera_config_.enabled)
    {
        Logger::error(
            "At least one camera must be enabled."
        );

        return false;
    }

    // Codec
    if (recording_config_.codec != "h264")
    {
        Logger::error(
            "Unsupported recording codec: " + recording_config_.codec
        );

        return false;
    }

    // Segment duration
    if (recording_config_.segment_duration <= 0)
    {
        Logger::error(
            "Invalid segment duration."
        );

        return false;
    }

    // Storage usage limit
    if (storage_config_.max_usage_percent <= 0 ||
        storage_config_.max_usage_percent > 100)
    {
        Logger::error(
            "Invalid storage usage limit."
        );

        return false;
    }

    // Recording path
    if (storage_config_.recording_path.empty())
    {
        Logger::error(
            "Recording path cannot be empty."
        );

        return false;
    }

    return true;
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