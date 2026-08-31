#pragma once

#include <string>

namespace bike_dashcam::platform {

struct PlatformInfo {
    std::string operating_system;
    std::string compiler;
    std::string architecture;
};

PlatformInfo detectPlatformInfo();

}  // namespace bike_dashcam::platform
