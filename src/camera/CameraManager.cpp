#include "CameraManager.hpp"

#include <iostream>

CameraManager::CameraManager(
    EventManager* event_manager)
    : event_manager_(event_manager)
{
    if (event_manager_ == nullptr)
    {
        std::cerr
            << "Warning: CameraManager created without EventManager."
            << std::endl;
    }
}

bool CameraManager::addCamera(
    const std::string& id,
    std::unique_ptr<Camera> camera)
{
    if (id.empty())
    {
        std::cerr << "Camera ID cannot be empty." << std::endl;
        return false;
    }

    if (!camera)
    {
        std::cerr << "Cannot add null camera." << std::endl;
        return false;
    }

    if (cameras_.find(id) != cameras_.end())
    {
        std::cerr << "Camera already exists: " << id << std::endl;
        return false;
    }

    cameras_[id] = std::move(camera);

    std::cout << "Camera added: " << id << std::endl;

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
        std::cout
            << "Starting camera: "
            << entry.first
            << std::endl;

        if (!entry.second->start())
        {
            std::cerr
                << "Failed to start camera: "
                << entry.first
                << std::endl;

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
        std::cout
            << "Stopping camera: "
            << entry.first
            << std::endl;

        if (!entry.second->stop())
        {
            std::cerr
                << "Failed to stop camera: "
                << entry.first
                << std::endl;

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
        std::cerr
            << "[CAMERA] Cannot recover unknown camera: "
            << id
            << std::endl;

        return false;
    }

    Camera* camera = it->second.get();

    if (camera == nullptr)
    {
        std::cerr
            << "[CAMERA] Camera object is null: "
            << id
            << std::endl;

        return false;
    }

    std::cout
        << "[CAMERA] Attempting recovery: "
        << id
        << std::endl;

    // Stop the camera first.
    if (!camera->stop())
    {
        std::cerr
            << "[CAMERA] Failed to stop camera during recovery: "
            << id
            << std::endl;

        return false;
    }

    // Start it again.
    if (!camera->start())
    {
        std::cerr
            << "[CAMERA] Failed to restart camera: "
            << id
            << std::endl;

        return false;
    }

    if (!camera->isHealthy())
    {
        std::cerr
            << "[CAMERA] Camera still unhealthy after recovery: "
            << id
            << std::endl;

        return false;
    }

    std::cout
        << "[CAMERA] Camera recovery successful: "
        << id
        << std::endl;

    return true;
}