#pragma once

#include <chrono>
#include <string>
#include <string_view>

namespace bike_dashcam::recording {

class SegmentManager {
public:
    bool setSegmentDuration(std::chrono::seconds duration);
    std::chrono::seconds segmentDuration() const;
    std::string buildSegmentFileName(
        std::string_view camera_id,
        std::chrono::system_clock::time_point start_time) const;

private:
    std::chrono::seconds duration_{300};
};

}  // namespace bike_dashcam::recording
