#pragma once

#include <string>

class SegmentManager
{
public:
    SegmentManager(
        const std::string& recording_path,
        const std::string& camera_id,
        int segment_duration);

    bool initialize();

    std::string getOutputPattern() const;

    int getSegmentDuration() const;

private:
    std::string recording_path_;
    std::string camera_id_;
    int segment_duration_;

    std::string camera_recording_path_;
};