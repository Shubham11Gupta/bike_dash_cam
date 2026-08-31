#include "system/Watchdog.hpp"

namespace bike_dashcam::system {

Watchdog::Watchdog(const std::chrono::seconds timeout)
    : timeout_(timeout), last_heartbeat_(std::chrono::steady_clock::now()) {
}

void Watchdog::noteHeartbeat() {
    last_heartbeat_ = std::chrono::steady_clock::now();
}

bool Watchdog::isHealthy() const {
    return std::chrono::steady_clock::now() - last_heartbeat_ <= timeout_;
}

}  // namespace bike_dashcam::system
