#pragma once

#include <string>
#include <vector>

namespace bike_dashcam::application {

struct ComponentStatus {
    std::string name;
    bool ok{false};
    std::string detail;
};

struct StartupReport {
    std::string version{"0.0.0"};
    std::string platform{"Unknown"};
    std::string mode{"Development"};
    std::vector<ComponentStatus> checks;
    bool ready{false};
};

}  // namespace bike_dashcam::application
