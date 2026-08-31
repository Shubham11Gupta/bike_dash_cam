#pragma once

#include <filesystem>
#include <string>

namespace bike_dashcam::configuration {

struct ApplicationSettings {
    std::string name{"BIKE DASHCAM POC-1"};
    std::string mode{"Development"};
    std::filesystem::path recordings_directory{"recordings"};
    std::filesystem::path logs_directory{"logs"};
};

struct CameraSettings {
    int expected_camera_count{2};
    int target_width{1920};
    int target_height{1080};
    int target_fps{30};
};

struct RecordingSettings {
    std::string codec{"H264"};
    int segment_duration_seconds{300};
    bool continuous{true};
};

struct LoggingSettings {
    std::string level{"INFO"};
    bool console_enabled{true};
};

struct DashcamConfig {
    ApplicationSettings application;
    CameraSettings cameras;
    RecordingSettings recording;
    LoggingSettings logging;
};

}  // namespace bike_dashcam::configuration
