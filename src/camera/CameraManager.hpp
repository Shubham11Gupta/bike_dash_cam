#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "Camera.hpp"
#include "../events/EventManager.hpp"

class CameraManager
{
public:
    explicit CameraManager(EventManager* event_manager);

    ~CameraManager() = default;

    bool addCamera(
        const std::string& id,
        std::unique_ptr<Camera> camera);

    Camera* getCamera(const std::string& id) const;

    bool startAll();
    bool stopAll();

    bool areAllHealthy() const;

    // Attempts to recover a specific camera.
    bool recoverCamera(const std::string& id);
    std::vector<std::string> getUnhealthyCameras() const;

private:
    std::unordered_map<
        std::string,
        std::unique_ptr<Camera>> cameras_;

    EventManager* event_manager_;
};