#include "CameraManager.hpp"

#include "../logging/Logger.hpp"

CameraManager::CameraManager(
    EventManager* event_manager)
    : event_manager_(event_manager)
{
    if (event_manager_ == nullptr)
    {
        Logger::warn(
            "CameraManager created without EventManager."
        );
    }
}

bool CameraManager::addCamera(
    const std::string& id,
    std::unique_ptr<Camera> camera)
{
    if (id.empty())
    {
        Logger::error("Camera ID cannot be empty.");
        return false;
    }

    if (!camera)
    {
        Logger::error("Cannot add null camera.");
        return false;
    }

    if (cameras_.find(id) != cameras_.end())
    {
        Logger::error("Camera already exists: " + id);
        return false;
    }

    cameras_[id] = std::move(camera);

    Logger::info("Camera added: " + id);

    return true;
}

Camera* CameraManager::getCamera(const std::string& id) const
{
    auto it = cameras_.find(id);

    if (it == cameras_.end())
    {
        return nullptr;
    }

    return it->second.get();
}

bool CameraManager::startAll()
{
    bool success = true;

    for (auto& entry : cameras_)
    {
        Logger::info(
            "Starting camera: " + entry.first
        );

        if (!entry.second->start())
        {
            Logger::error(
                "Failed to start camera: " + entry.first
            );

            if (event_manager_ != nullptr)
            {
                event_manager_->publish(
                    EventType::CAMERA_DISCONNECTED,
                    entry.first,
                    "Camera failed to start."
                );
            }

            success = false;
        }
        else
        {
            if (event_manager_ != nullptr)
            {
                event_manager_->publish(
                    EventType::CAMERA_CONNECTED,
                    entry.first,
                    "Camera started successfully."
                );
            }
        }
    }

    return success;
}

bool CameraManager::stopAll()
{
    bool success = true;

    for (auto& entry : cameras_)
    {
        Logger::info(
            "Stopping camera: " + entry.first
        );

        if (!entry.second->stop())
        {
            Logger::error(
                "Failed to stop camera: " + entry.first
            );

            if (event_manager_ != nullptr)
            {
                event_manager_->publish(
                    EventType::CAMERA_DISCONNECTED,
                    entry.first,
                    "Camera failed to stop cleanly."
                );
            }

            success = false;
        }
        else
        {
            if (event_manager_ != nullptr)
            {
                event_manager_->publish(
                    EventType::CAMERA_DISCONNECTED,
                    entry.first,
                    "Camera stopped."
                );
            }
        }
    }

    return success;
}

bool CameraManager::areAllHealthy() const
{
    if (cameras_.empty())
    {
        return false;
    }

    for (const auto& entry : cameras_)
    {
        if (!entry.second->isHealthy())
        {
            return false;
        }
    }

    return true;
}

std::vector<std::string> CameraManager::getUnhealthyCameras() const
{
    std::vector<std::string> unhealthy_cameras;

    for (const auto& entry : cameras_)
    {
        if (!entry.second->isHealthy())
        {
            unhealthy_cameras.push_back(entry.first);
        }
    }

    return unhealthy_cameras;
}

bool CameraManager::recoverCamera(const std::string& id)
{
    auto it = cameras_.find(id);

    if (it == cameras_.end())
    {
        Logger::error(
            "[CAMERA] Cannot recover unknown camera: " + id
        );

        return false;
    }

    Camera* camera = it->second.get();

    if (camera == nullptr)
    {
        Logger::error(
            "[CAMERA] Camera object is null: " + id
        );

        return false;
    }

    Logger::info(
        "[CAMERA] Attempting recovery: " + id
    );

    // Stop the camera first.
    if (!camera->stop())
    {
        Logger::error(
            "[CAMERA] Failed to stop camera during recovery: " + id
        );

        return false;
    }

    // Start it again.
    if (!camera->start())
    {
        Logger::error(
            "[CAMERA] Failed to restart camera: " + id
        );

        return false;
    }

    if (!camera->isHealthy())
    {
        Logger::error(
            "[CAMERA] Camera still unhealthy after recovery: " + id
        );

        return false;
    }

    Logger::info(
        "[CAMERA] Camera recovery successful: " + id
    );

    return true;
}