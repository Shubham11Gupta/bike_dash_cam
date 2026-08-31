#pragma once

#include "system/ISystemMonitor.hpp"

namespace bike_dashcam::system {

class SystemMonitor : public ISystemMonitor {
public:
    bool initialize(std::string& error_message) override;
    bool isReady() const override;
    SystemSnapshot snapshot() const override;

private:
    bool ready_{false};
};

}  // namespace bike_dashcam::system
