#include "recording/SegmentManager.hpp"

#include <algorithm>
#include <cctype>
#include <ctime>
#include <iomanip>
#include <sstream>

namespace bike_dashcam::recording {
namespace {

std::string sanitizeToken(std::string_view input) {
    std::string sanitized(input);
    std::replace_if(
        sanitized.begin(),
        sanitized.end(),
        [](const char character) {
            return !std::isalnum(static_cast<unsigned char>(character)) && character != '_' && character != '-';
        },
        '_');
    return sanitized;
}

}  // namespace

bool SegmentManager::setSegmentDuration(const std::chrono::seconds duration) {
    if (duration.count() <= 0) {
        return false;
    }

    duration_ = duration;
    return true;
}

std::chrono::seconds SegmentManager::segmentDuration() const {
    return duration_;
}

std::string SegmentManager::buildSegmentFileName(
    const std::string_view camera_id,
    const std::chrono::system_clock::time_point start_time) const {
    const std::time_t start_time_value = std::chrono::system_clock::to_time_t(start_time);

    std::tm local_time{};
#if defined(_WIN32)
    localtime_s(&local_time, &start_time_value);
#else
    localtime_r(&start_time_value, &local_time);
#endif

    std::ostringstream stream;
    stream << sanitizeToken(camera_id) << '_' << std::put_time(&local_time, "%Y%m%d_%H%M%S") << ".mp4";
    return stream.str();
}

}  // namespace bike_dashcam::recording
