#include "system/SystemMonitor.hpp"

namespace bike_dashcam::system {

bool SystemMonitor::initialize(std::string& error_message) {
    error_message.clear();
    ready_ = true;
    return true;
}

bool SystemMonitor::isReady() const {
    return ready_;
}

SystemSnapshot SystemMonitor::snapshot() const {
    return {};
}

}  // namespace bike_dashcam::system
