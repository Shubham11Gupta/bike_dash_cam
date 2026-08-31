#include "camera/CameraBackendFactory.hpp"

#ifdef _WIN32
#include "camera/windows/WindowsMediaFoundationCameraBackend.hpp"
#endif

namespace bike_dashcam::camera {

std::vector<std::shared_ptr<ICameraBackend>> createPlatformCameraBackends() {
#ifdef _WIN32
    return {std::make_shared<WindowsMediaFoundationCameraBackend>()};
#else
    return {};
#endif
}

}  // namespace bike_dashcam::camera
