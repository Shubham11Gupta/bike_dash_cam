#include "SegmentManager.hpp"

#include <filesystem>
#include <iostream>

SegmentManager::SegmentManager(
    const std::string& recording_path,
    const std::string& camera_id,
    int segment_duration)
    : recording_path_(recording_path),
      camera_id_(camera_id),
      segment_duration_(segment_duration)
{
}

bool SegmentManager::initialize()
{
    if (recording_path_.empty())
    {
        std::cerr << "Recording path cannot be empty."
                  << std::endl;

        return false;
    }

    if (camera_id_.empty())
    {
        std::cerr << "Camera ID cannot be empty."
                  << std::endl;

        return false;
    }

    if (segment_duration_ <= 0)
    {
        std::cerr << "Segment duration must be greater than zero."
                  << std::endl;

        return false;
    }

    camera_recording_path_ =
        recording_path_ + "/" + camera_id_;

    try
    {
        std::filesystem::create_directories(
            camera_recording_path_);
    }
    catch (const std::filesystem::filesystem_error& error)
    {
        std::cerr << "Failed to create recording directory: "
                  << error.what()
                  << std::endl;

        return false;
    }

    std::cout << "Segment manager initialized for: "
              << camera_id_
              << std::endl;

    std::cout << "Recording directory: "
              << camera_recording_path_
              << std::endl;

    return true;
}

std::string SegmentManager::getOutputPattern() const
{
    return camera_recording_path_ +
           "/segment-%02d.mp4";
}

int SegmentManager::getSegmentDuration() const
{
    return segment_duration_;
}