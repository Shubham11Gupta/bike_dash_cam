#include "camera/CameraBackendFactory.hpp"
#include "camera/DummyCameraBackend.hpp"

#ifdef _WIN32
#include "camera/windows/WindowsMediaFoundationCameraBackend.hpp"
#endif

namespace bike_dashcam::camera {

std::vector<std::shared_ptr<ICameraBackend>> createPlatformCameraBackends() {
#ifdef _WIN32
    return {std::make_shared<WindowsMediaFoundationCameraBackend>(), std::make_shared<DummyCameraBackend>()};
#else
    return {std::make_shared<DummyCameraBackend>()};
#endif
}

}  // namespace bike_dashcam::camera
