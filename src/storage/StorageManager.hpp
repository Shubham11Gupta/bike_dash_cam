#pragma once

#include <cstdint>
#include <filesystem>
#include <string>

class StorageManager
{
public:
    explicit StorageManager(
        const std::string& recording_path,
        int max_usage_percent
    );

    bool initialize();

    bool isWritable() const;

    std::uintmax_t getTotalSpace() const;
    std::uintmax_t getAvailableSpace() const;
    std::uintmax_t getUsedSpace() const;

    double getUsagePercent() const;

    bool isStorageLimitReached() const;
    bool deleteOldestSegment();
    bool enforceStorageLimit();

    const std::filesystem::path& getRecordingPath() const;
    // POC test helper.
    // Simulates storage limit being reached without filling the disk.
    bool simulateStorageLimit();
    bool simulateStorageDeletionFailure();

private:
    std::filesystem::path recording_path_;
    int max_usage_percent_;

    bool initialized_;
    bool simulated_storage_limit_;
    bool simulated_deletion_failure_;
};