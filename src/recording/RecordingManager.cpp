#include "RecordingManager.hpp"

#include <iostream>

RecordingManager::RecordingManager(
    StorageManager* storage_manager,
    EventManager* event_manager)
    : storage_manager_(storage_manager),
      event_manager_(event_manager)
{
    if (storage_manager_ == nullptr)
    {
        std::cerr
            << "Warning: RecordingManager created without StorageManager."
            << std::endl;
    }

    if (event_manager_ == nullptr)
    {
        std::cerr
            << "Warning: RecordingManager created without EventManager."
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
        else
        {
            if (event_manager_ != nullptr)
            {
                event_manager_->publish(
                    EventType::RECORDING_STARTED,
                    entry.first,
                    "Recording started successfully."
                );
            }
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
        else
        {
            if (event_manager_ != nullptr)
            {
                event_manager_->publish(
                    EventType::RECORDING_STOPPED,
                    entry.first,
                    "Recording stopped successfully."
                );
            }
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
bool RecordingManager::recoverRecorder(const std::string& id)
{
    auto it = recorders_.find(id);

    if (it == recorders_.end())
    {
        std::cerr
            << "[RECORDING] Recorder not found: "
            << id
            << std::endl;

        return false;
    }

    Recorder* recorder = it->second.get();

    if (recorder == nullptr)
    {
        std::cerr
            << "[RECORDING] Recorder is null: "
            << id
            << std::endl;

        return false;
    }

    std::cout
        << "[RECORDING] Recovering recorder: "
        << id
        << std::endl;

    // ------------------------------------------------------------
    // Stop the existing recorder if it is still running.
    // ------------------------------------------------------------

    if (recorder->isRunning())
    {
        if (!recorder->stop())
        {
            std::cerr
                << "[RECORDING] Failed to stop recorder during recovery: "
                << id
                << std::endl;

            return false;
        }
    }
    else
    {
        std::cout
            << "[RECORDING] Recorder is already stopped: "
            << id
            << std::endl;
    }

    // ------------------------------------------------------------
    // Start a fresh recording pipeline.
    // ------------------------------------------------------------

    if (!recorder->start())
    {
        std::cerr
            << "[RECORDING] Failed to restart recorder: "
            << id
            << std::endl;

        return false;
    }

    std::cout
        << "[RECORDING] Recorder recovery successful: "
        << id
        << std::endl;

    if (event_manager_ != nullptr)
    {
        event_manager_->publish(
            EventType::RECORDING_STARTED,
            id,
            "Recording restarted successfully after recovery."
        );
    }

    return true;
}

std::vector<std::string>
RecordingManager::getUnhealthyRecorders() const
{
    std::vector<std::string> unhealthy_recorders;

    for (const auto& entry : recorders_)
    {
        const std::string& id = entry.first;
        const Recorder* recorder = entry.second.get();

        if (recorder == nullptr || !recorder->isRunning())
        {
            unhealthy_recorders.push_back(id);
        }
    }

    return unhealthy_recorders;
}

bool RecordingManager::simulateRecorderFailure(
    const std::string& id)
{
    auto it = recorders_.find(id);

    if (it == recorders_.end())
    {
        std::cerr
            << "[RECORDING TEST] Recorder not found: "
            << id
            << std::endl;

        return false;
    }

    Recorder* recorder = it->second.get();

    if (recorder == nullptr)
    {
        std::cerr
            << "[RECORDING TEST] Recorder is null: "
            << id
            << std::endl;

        return false;
    }

    return recorder->simulateFailure();
}