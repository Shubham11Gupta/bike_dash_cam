#pragma once

#include "camera/ICameraBackend.hpp"

namespace bike_dashcam::camera {
class DummyCameraBackend final : public ICameraBackend {
public:
    std::string backendName() const override;
    std::vector<CameraDescriptor> discover() override;
    bool initialize(const CameraDescriptor&, std::string& error_message) override;
    bool captureFor(const CameraDescriptor&, std::chrono::milliseconds duration,
                    CaptureStatistics& statistics, std::string& error_message) override;
};
}  // namespace bike_dashcam::camera
