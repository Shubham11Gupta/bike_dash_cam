#pragma once

#include <memory>
#include <string>
#include <unordered_map>

#include "Camera.hpp"

class CameraManager
{
public:
    CameraManager() = default;
    ~CameraManager() = default;

    bool addCamera(const std::string& id, std::unique_ptr<Camera> camera);

    Camera* getCamera(const std::string& id) const;

    bool startAll();
    bool stopAll();

    bool areAllHealthy() const;

private:
    std::unordered_map<std::string, std::unique_ptr<Camera>> cameras_;
};