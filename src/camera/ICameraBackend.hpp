#pragma once

#include <chrono>
#include <cstddef>
#include <string>
#include <vector>

namespace bike_dashcam::camera {

struct CameraDescriptor {
    std::string id;
    std::string name;
    std::string backend_name;
    bool available{true};
};

struct CaptureStatistics {
    std::string camera_name;
    std::size_t frame_count{0};
    std::size_t dropped_frame_count{0};
    double fps{0.0};
    bool healthy{false};
};

class ICameraBackend {
public:
    virtual ~ICameraBackend() = default;

    virtual std::string backendName() const = 0;
    virtual std::vector<CameraDescriptor> discover() = 0;
    virtual bool initialize(const CameraDescriptor& camera, std::string& error_message) = 0;
    virtual bool captureFor(const CameraDescriptor& camera, std::chrono::milliseconds duration,
                            CaptureStatistics& statistics, std::string& error_message) = 0;
};

}  // namespace bike_dashcam::camera
