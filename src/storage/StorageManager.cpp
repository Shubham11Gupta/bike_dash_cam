#include "StorageManager.hpp"

#include <fstream>
#include <iostream>
#include <system_error>

#include "../logging/Logger.hpp"

StorageManager::StorageManager(
    const std::string& recording_path,
    int max_usage_percent
)
    : recording_path_(recording_path),
      max_usage_percent_(max_usage_percent),
      initialized_(false),
      simulated_storage_limit_(false),
      simulated_deletion_failure_(false)
{
}

bool StorageManager::initialize()
{
    try
    {
        if (!std::filesystem::exists(recording_path_))
        {
            std::filesystem::create_directories(recording_path_);

            Logger::info(
                "Created storage directory: " +
                recording_path_.string()
            );
        }

        if (!std::filesystem::is_directory(recording_path_))
        {
            Logger::error(
                "Storage path is not a directory: " +
                recording_path_.string()
            );

            return false;
        }

        if (!isWritable())
        {
            Logger::error(
                "Storage path is not writable: " +
                recording_path_.string()
            );

            return false;
        }

        initialized_ = true;

        Logger::info(
            "Storage manager initialized."
        );

        Logger::info(
            "Storage path: " +
            recording_path_.string()
        );

        return true;
    }
    catch (const std::filesystem::filesystem_error& e)
    {
        Logger::error(
            "Storage initialization failed: " +
            std::string(e.what())
        );

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
    if (simulated_storage_limit_)
    {
        return true;
    }

    return getUsagePercent() >=
           static_cast<double>(max_usage_percent_);
}


// ============================================================
// Delete Oldest Segment
// ============================================================

bool StorageManager::deleteOldestSegment()
{
    if (simulated_deletion_failure_)
    {
        Logger::warn(
            "[STORAGE TEST] Simulating inability to delete segments."
        );

        return false;
    }
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
            Logger::warn(
                "No recording segments found for deletion."
            );

            return false;
        }

        Logger::info(
            "Deleting oldest segment: " +
            oldest_file.string()
        );

        if (std::filesystem::remove(oldest_file))
        {
            Logger::info(
                "Oldest segment deleted successfully."
            );

            if (simulated_storage_limit_)
            {
                simulated_storage_limit_ = false;

                Logger::info(
                    "[STORAGE TEST] Simulated storage pressure cleared."
                );
            }

            return true;
        }

        return false;
    }
    catch (const std::filesystem::filesystem_error& e)
    {
       Logger::error(
            "Failed to delete oldest segment: " +
            std::string(e.what())
        );

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
        Logger::error(
            "Storage manager is not initialized."
        );

        return false;
    }

    Logger::debug(
        "Checking storage limit."
    );

    while (isStorageLimitReached())
    {
        Logger::warn(
            "Storage limit reached."
        );

        Logger::info(
            "Current storage usage: " +
            std::to_string(getUsagePercent()) +
            "%"
        );

        if (!deleteOldestSegment())
        {
            Logger::error(
                "Unable to free storage."
            );

            return false;
        }
    }

    Logger::debug(
        "Storage usage is within configured limit: " +
        std::to_string(getUsagePercent()) +
        "%"
    );

    return true;
}

const std::filesystem::path&
StorageManager::getRecordingPath() const
{
    return recording_path_;
}

bool StorageManager::simulateStorageLimit()
{
    if (!initialized_)
    {
        Logger::error(
            "[STORAGE TEST] Storage manager is not initialized."
        );

        return false;
    }

    Logger::info(
        "[STORAGE TEST] Simulating storage limit."
    );

    simulated_storage_limit_ = true;

    Logger::info(
        "[STORAGE TEST] Storage limit simulated."
    );

    return true;
}

bool StorageManager::simulateStorageDeletionFailure()
{
    if (!initialized_)
    {
        Logger::error(
            "[STORAGE TEST] Storage manager is not initialized."
        );

        return false;
    }

    Logger::info(
        "[STORAGE TEST] Simulating storage deletion failure."
    );

    simulated_deletion_failure_ = true;

    Logger::info(
        "[STORAGE TEST] Storage deletion failure simulated."
    );

    return true;
}