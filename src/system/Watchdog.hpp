#pragma once

#include <chrono>

namespace bike_dashcam::system {

class Watchdog {
public:
    explicit Watchdog(std::chrono::seconds timeout = std::chrono::seconds{5});

    void noteHeartbeat();
    bool isHealthy() const;

private:
    std::chrono::seconds timeout_;
    std::chrono::steady_clock::time_point last_heartbeat_;
};

}  // namespace bike_dashcam::system
