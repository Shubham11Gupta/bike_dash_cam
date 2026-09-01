#include "camera/DummyCameraBackend.hpp"

#include <thread>

namespace bike_dashcam::camera {
std::string DummyCameraBackend::backendName() const { return "Dummy camera"; }
std::vector<CameraDescriptor> DummyCameraBackend::discover() {
    return {{"dummy-camera-2", "Dummy Camera 2", backendName(), true}};
}
bool DummyCameraBackend::initialize(const CameraDescriptor&, std::string& error_message) { error_message.clear(); return true; }
bool DummyCameraBackend::captureFor(const CameraDescriptor& camera, const std::chrono::milliseconds duration,
                                    CaptureStatistics& statistics, std::string& error_message) {
    error_message.clear(); statistics = CaptureStatistics{}; statistics.camera_name = camera.name;
    std::this_thread::sleep_for(duration);
    statistics.frame_count = static_cast<std::size_t>(duration.count() * 30 / 1000);
    statistics.fps = 30.0; statistics.healthy = true; return true;
}
}  // namespace bike_dashcam::camera
