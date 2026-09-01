#pragma once

#include "camera/ICameraBackend.hpp"
#include "logging/Logger.hpp"

#include <memory>
#include <chrono>
#include <string>
#include <string_view>
#include <vector>

namespace bike_dashcam::camera {

class CameraManager {
public:
    explicit CameraManager(logging::Logger& logger);

    void registerBackend(std::shared_ptr<ICameraBackend> backend);
    bool initialize(
        std::size_t required_camera_count,
        std::string_view preferred_camera_name,
        std::string_view ignored_camera_name,
        std::string& error_message);
    bool captureFor(std::chrono::milliseconds duration, std::vector<CaptureStatistics>& statistics,
                    std::string& error_message);

    std::size_t backendCount() const;
    std::size_t discoveredCameraCount() const;
    std::size_t initializedCameraCount() const;
    const std::vector<CameraDescriptor>& discoveredCameras() const;
    const std::vector<CameraDescriptor>& initializedCameras() const;

private:
    logging::Logger& logger_;
    std::vector<std::shared_ptr<ICameraBackend>> backends_;
    std::vector<CameraDescriptor> discovered_cameras_;
    std::vector<CameraDescriptor> initialized_cameras_;
};

}  // namespace bike_dashcam::camera
