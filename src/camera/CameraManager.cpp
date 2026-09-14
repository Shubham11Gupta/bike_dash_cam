#include "CameraManager.hpp"

#include <iostream>

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
        std::cout << "Starting camera: "
                  << entry.first
                  << std::endl;

        if (!entry.second->start())
        {
            std::cerr << "Failed to start camera: "
                      << entry.first
                      << std::endl;

            success = false;
        }
    }

    return success;
}

bool CameraManager::stopAll()
{
    bool success = true;

    for (auto& entry : cameras_)
    {
        std::cout << "Stopping camera: "
                  << entry.first
                  << std::endl;

        if (!entry.second->stop())
        {
            std::cerr << "Failed to stop camera: "
                      << entry.first
                      << std::endl;

            success = false;
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