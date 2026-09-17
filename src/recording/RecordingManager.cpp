#include "RecordingManager.hpp"

#include <iostream>

RecordingManager::RecordingManager(
    StorageManager* storage_manager)
    : storage_manager_(storage_manager)
{
    if (storage_manager_ == nullptr)
    {
        std::cerr
            << "Warning: RecordingManager created without StorageManager."
            << std::endl;
    }
}

bool RecordingManager::addRecorder(
    const std::string& id,
    std::unique_ptr<Recorder> recorder)
{
    if (id.empty())
    {
        std::cerr
            << "Recorder ID cannot be empty."
            << std::endl;

        return false;
    }

    if (!recorder)
    {
        std::cerr
            << "Cannot add null recorder."
            << std::endl;

        return false;
    }

    if (recorders_.find(id) != recorders_.end())
    {
        std::cerr
            << "Recorder already exists: "
            << id
            << std::endl;

        return false;
    }

    recorders_[id] = std::move(recorder);

    std::cout
        << "Recorder added: "
        << id
        << std::endl;

    return true;
}

Recorder* RecordingManager::getRecorder(
    const std::string& id) const
{
    auto it = recorders_.find(id);

    if (it == recorders_.end())
    {
        return nullptr;
    }

    return it->second.get();
}

bool RecordingManager::startAll()
{
    bool success = true;

    for (auto& entry : recorders_)
    {
        std::cout
            << "Starting recorder: "
            << entry.first
            << std::endl;

        if (!entry.second->start())
        {
            std::cerr
                << "Failed to start recorder: "
                << entry.first
                << std::endl;

            success = false;
        }
    }

    return success;
}

bool RecordingManager::stopAll()
{
    bool success = true;

    for (auto& entry : recorders_)
    {
        std::cout
            << "Stopping recorder: "
            << entry.first
            << std::endl;

        if (!entry.second->stop())
        {
            success = false;
        }
    }

    return success;
}

bool RecordingManager::enforceStorageLimit()
{
    if (storage_manager_ == nullptr)
    {
        std::cerr
            << "Cannot enforce storage limit: "
            << "StorageManager is not available."
            << std::endl;

        return false;
    }

    std::cout
        << "Checking storage limit..."
        << std::endl;

    while (storage_manager_->isStorageLimitReached())
    {
        std::cout
            << "Storage limit reached."
            << std::endl;

        std::cout
            << "Current storage usage: "
            << storage_manager_->getUsagePercent()
            << "%"
            << std::endl;

        if (!storage_manager_->deleteOldestSegment())
        {
            std::cerr
                << "Unable to free storage."
                << std::endl;

            return false;
        }
    }

    std::cout
        << "Storage usage is within configured limit: "
        << storage_manager_->getUsagePercent()
        << "%"
        << std::endl;

    return true;
}

bool RecordingManager::monitorStorage()
{
    if (storage_manager_ == nullptr)
    {
        std::cerr
            << "Cannot monitor storage: "
            << "StorageManager is not available."
            << std::endl;

        return false;
    }

    const double usage =
        storage_manager_->getUsagePercent();

    std::cout
        << "Storage monitor: "
        << usage
        << "%"
        << std::endl;

    if (storage_manager_->isStorageLimitReached())
    {
        std::cout
            << "Storage limit reached. "
            << "Enforcing storage policy..."
            << std::endl;

        return enforceStorageLimit();
    }

    return true;
}