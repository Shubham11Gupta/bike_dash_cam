#pragma once

#include <string>

namespace bike_dashcam::events {

enum class EventSeverity {
    Info,
    Warning,
    Critical
};

struct Event {
    std::string type;
    EventSeverity severity{EventSeverity::Info};
    std::string source;
    std::string message;
};

}  // namespace bike_dashcam::events
