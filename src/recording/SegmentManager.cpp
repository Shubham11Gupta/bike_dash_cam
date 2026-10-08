#include "SegmentManager.hpp"

#include <filesystem>
#include <iostream>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>

SegmentManager::SegmentManager(
    const std::string& recording_path,
    const std::string& camera_id,
    int segment_duration)
    : recording_path_(recording_path),
      camera_id_(camera_id),
      segment_duration_(segment_duration)
{
}

std::string SegmentManager::getCameraId() const
{
    return camera_id_;
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

int SegmentManager::getSegmentDuration() const
{
    return segment_duration_;
}

std::string SegmentManager::getSegmentPath() const
{
    auto now = std::chrono::system_clock::now();
    std::time_t current_time =
        std::chrono::system_clock::to_time_t(now);

    std::tm local_time{};

#ifdef _WIN32
    localtime_s(&local_time, &current_time);
#else
    localtime_r(&current_time, &local_time);
#endif

    std::ostringstream timestamp;

    timestamp
        << std::put_time(
            &local_time,
            "%Y%m%d_%H%M%S"
        );

    return camera_recording_path_ +
           "/" +
           timestamp.str() +
           ".mp4";
}