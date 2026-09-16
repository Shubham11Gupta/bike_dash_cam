#include "StorageManager.hpp"

#include <fstream>
#include <iostream>
#include <system_error>

StorageManager::StorageManager(
    const std::string& recording_path,
    int max_usage_percent
)
    : recording_path_(recording_path),
      max_usage_percent_(max_usage_percent),
      initialized_(false)
{
}

bool StorageManager::initialize()
{
    try
    {
        if (!std::filesystem::exists(recording_path_))
        {
            std::filesystem::create_directories(recording_path_);
        }

        if (!std::filesystem::is_directory(recording_path_))
        {
            std::cerr
                << "Storage path is not a directory: "
                << recording_path_
                << std::endl;

            return false;
        }

        if (!isWritable())
        {
            std::cerr
                << "Storage path is not writable: "
                << recording_path_
                << std::endl;

            return false;
        }

        initialized_ = true;

        std::cout
            << "Storage manager initialized."
            << std::endl;

        std::cout
            << "Storage path: "
            << recording_path_
            << std::endl;

        return true;
    }
    catch (const std::filesystem::filesystem_error& e)
    {
        std::cerr
            << "Storage initialization failed: "
            << e.what()
            << std::endl;

        return false;
    }
}

bool StorageManager::isWritable() const
{
    try
    {
        if (!std::filesystem::exists(recording_path_))
        {
            return false;
        }

        const auto test_file =
            recording_path_ / ".storage_write_test";

        {
            std::ofstream file(test_file);

            if (!file.is_open())
            {
                return false;
            }

            file << "storage test";
        }

        std::filesystem::remove(test_file);

        return true;
    }
    catch (...)
    {
        return false;
    }
}

std::uintmax_t StorageManager::getTotalSpace() const
{
    try
    {
        const auto space_info =
            std::filesystem::space(recording_path_);

        return space_info.capacity;
    }
    catch (...)
    {
        return 0;
    }
}

std::uintmax_t StorageManager::getAvailableSpace() const
{
    try
    {
        const auto space_info =
            std::filesystem::space(recording_path_);

        return space_info.available;
    }
    catch (...)
    {
        return 0;
    }
}

std::uintmax_t StorageManager::getUsedSpace() const
{
    const auto total = getTotalSpace();
    const auto available = getAvailableSpace();

    if (total == 0 || available > total)
    {
        return 0;
    }

    return total - available;
}

double StorageManager::getUsagePercent() const
{
    const auto total = getTotalSpace();
    const auto available = getAvailableSpace();

    if (total == 0 || available > total)
    {
        return 0.0;
    }

    const auto used = total - available;

    return (
        static_cast<double>(used) /
        static_cast<double>(total)
    ) * 100.0;
}


// ============================================================
// Storage Limit Check
// ============================================================

bool StorageManager::isStorageLimitReached() const
{
    return getUsagePercent() >=
           static_cast<double>(max_usage_percent_);
}


// ============================================================
// Delete Oldest Segment
// ============================================================

bool StorageManager::deleteOldestSegment()
{
    try
    {
        if (!std::filesystem::exists(recording_path_))
        {
            return false;
        }

        std::filesystem::path oldest_file;
        std::filesystem::file_time_type oldest_time;

        bool found_segment = false;

        /*
         * Search recursively because recordings are organized as:
         *
         * video_recordings/
         * ├── front/
         * │   ├── segment-00.mp4
         * │   ├── segment-01.mp4
         * │   └── ...
         * │
         * └── rear/
         *     ├── segment-00.mp4
         *     ├── segment-01.mp4
         *     └── ...
         */

        for (const auto& entry :
             std::filesystem::recursive_directory_iterator(
                 recording_path_))
        {
            if (!entry.is_regular_file())
            {
                continue;
            }

            const auto& path = entry.path();

            /*
             * For now we only consider MP4 recording segments.
             */
            if (path.extension() != ".mp4")
            {
                continue;
            }

            const auto filename =
                path.filename().string();

            /*
             * Only delete files that follow our
             * segment naming convention.
             *
             * Example:
             * segment-00.mp4
             * segment-01.mp4
             */

            if (filename.rfind("segment-", 0) != 0)
            {
                continue;
            }

            const auto write_time =
                std::filesystem::last_write_time(path);

            if (!found_segment ||
                write_time < oldest_time)
            {
                oldest_file = path;
                oldest_time = write_time;
                found_segment = true;
            }
        }

        if (!found_segment)
        {
            std::cout
                << "No recording segments found for deletion."
                << std::endl;

            return false;
        }

        std::cout
            << "Deleting oldest segment: "
            << oldest_file
            << std::endl;

        if (std::filesystem::remove(oldest_file))
        {
            std::cout
                << "Oldest segment deleted successfully."
                << std::endl;

            return true;
        }

        return false;
    }
    catch (const std::filesystem::filesystem_error& e)
    {
        std::cerr
            << "Failed to delete oldest segment: "
            << e.what()
            << std::endl;

        return false;
    }
}

// ============================================================
// Enforce Storage Limit
// ============================================================

bool StorageManager::enforceStorageLimit()
{
    if (!initialized_)
    {
        std::cerr
            << "Storage manager is not initialized."
            << std::endl;

        return false;
    }

    std::cout
        << "Checking storage limit..."
        << std::endl;

    while (isStorageLimitReached())
    {
        std::cout
            << "Storage limit reached."
            << std::endl;

        std::cout
            << "Current storage usage: "
            << getUsagePercent()
            << "%"
            << std::endl;

        if (!deleteOldestSegment())
        {
            std::cerr
                << "Unable to free storage."
                << std::endl;

            return false;
        }
    }

    std::cout
        << "Storage usage is within configured limit: "
        << getUsagePercent()
        << "%"
        << std::endl;

    return true;
}

const std::filesystem::path&
StorageManager::getRecordingPath() const
{
    return recording_path_;
}