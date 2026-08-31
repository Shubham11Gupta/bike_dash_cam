#pragma once

#include <cstdint>
#include <string>

namespace bike_dashcam::system {

struct SystemSnapshot {
    double cpu_percent{0.0};
    std::uint64_t memory_megabytes{0};
    bool metrics_available{false};
    std::string note{"Metrics not yet implemented in Milestone 1."};
};

class ISystemMonitor {
public:
    virtual ~ISystemMonitor() = default;

    virtual bool initialize(std::string& error_message) = 0;
    virtual bool isReady() const = 0;
    virtual SystemSnapshot snapshot() const = 0;
};

}  // namespace bike_dashcam::system
