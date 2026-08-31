#pragma once

#include "camera/ICameraBackend.hpp"

#include <memory>
#include <vector>

namespace bike_dashcam::camera {

std::vector<std::shared_ptr<ICameraBackend>> createPlatformCameraBackends();

}  // namespace bike_dashcam::camera
