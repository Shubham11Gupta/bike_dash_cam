#pragma once

#include "camera/ICameraBackend.hpp"

namespace bike_dashcam::camera {

class WindowsMediaFoundationCameraBackend final : public ICameraBackend {
public:
    std::string backendName() const override;
    std::vector<CameraDescriptor> discover() override;
    bool initialize(const CameraDescriptor& camera, std::string& error_message) override;
};

}  // namespace bike_dashcam::camera
