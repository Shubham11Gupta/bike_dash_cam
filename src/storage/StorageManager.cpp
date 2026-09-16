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

bool StorageManager::isStorageLimitReached() const
{
    return getUsagePercent() >=
           static_cast<double>(max_usage_percent_);
}

bool StorageManager::deleteOldestSegment()
{
    try
    {
        if (!initialized_)
        {
            std::cerr
                << "Storage manager is not initialized."
                << std::endl;

            return false;
        }

        std::filesystem::path oldest_file;
        std::filesystem::file_time_type oldest_time;

        bool found_file = false;

        for (const auto& entry :
             std::filesystem::recursive_directory_iterator(
                 recording_path_))
        {
            if (!entry.is_regular_file())
            {
                continue;
            }

            const auto extension =
                entry.path().extension().string();

            if (extension != ".mp4")
            {
                continue;
            }

            const auto current_time =
                std::filesystem::last_write_time(
                    entry.path());

            if (!found_file ||
                current_time < oldest_time)
            {
                oldest_file = entry.path();
                oldest_time = current_time;
                found_file = true;
            }
        }

        if (!found_file)
        {
            std::cout
                << "No recording segments found for cleanup."
                << std::endl;

            return false;
        }

        std::cout
            << "Deleting oldest segment: "
            << oldest_file
            << std::endl;

        if (!std::filesystem::remove(oldest_file))
        {
            std::cerr
                << "Failed to delete segment: "
                << oldest_file
                << std::endl;

            return false;
        }

        std::cout
            << "Oldest segment deleted successfully."
            << std::endl;

        return true;
    }
    catch (const std::filesystem::filesystem_error& e)
    {
        std::cerr
            << "Storage cleanup failed: "
            << e.what()
            << std::endl;

        return false;
    }
}

const std::filesystem::path&
StorageManager::getRecordingPath() const
{
    return recording_path_;
}