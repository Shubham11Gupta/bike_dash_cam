#pragma once

#include "camera/ICameraBackend.hpp"
#include "logging/Logger.hpp"

#include <memory>
#include <string>
#include <vector>

namespace bike_dashcam::camera {

class CameraManager {
public:
    explicit CameraManager(logging::Logger& logger);

    void registerBackend(std::shared_ptr<ICameraBackend> backend);
    bool initialize(std::string& error_message);

    std::size_t backendCount() const;
    std::size_t discoveredCameraCount() const;
    const std::vector<CameraDescriptor>& discoveredCameras() const;

private:
    logging::Logger& logger_;
    std::vector<std::shared_ptr<ICameraBackend>> backends_;
    std::vector<CameraDescriptor> discovered_cameras_;
};

}  // namespace bike_dashcam::camera
