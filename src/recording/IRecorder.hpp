#pragma once

#include <string>

namespace bike_dashcam::recording {

struct RecordingProfile {
    int width{1920};
    int height{1080};
    int fps{30};
    std::string codec{"H264"};
    int segment_duration_seconds{300};
    bool continuous{true};
};

class IRecorder {
public:
    virtual ~IRecorder() = default;

    virtual std::string recorderName() const = 0;
    virtual bool prepare(const RecordingProfile& profile, std::string& error_message) = 0;
};

}  // namespace bike_dashcam::recording
