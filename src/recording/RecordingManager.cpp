#include "RecordingManager.hpp"

#include "../logging/Logger.hpp"

RecordingManager::RecordingManager(
    StorageManager* storage_manager,
    EventManager* event_manager)
    : storage_manager_(storage_manager),
      event_manager_(event_manager)
{
    if (storage_manager_ == nullptr)
    {
        Logger::warn(
            "RecordingManager created without StorageManager."
        );
    }

    if (event_manager_ == nullptr)
    {
        Logger::warn(
            "RecordingManager created without EventManager."
        );
    }
}

bool RecordingManager::addRecorder(
    const std::string& id,
    std::unique_ptr<Recorder> recorder)
{
    if (id.empty())
    {
        Logger::error(
            "Recorder ID cannot be empty."
        );

        return false;
    }

    if (!recorder)
    {
        Logger::error(
            "Cannot add null recorder."
        );

        return false;
    }

    if (recorders_.find(id) != recorders_.end())
    {
        Logger::error(
            "Recorder already exists: " + id
        );

        return false;
    }

    recorders_[id] = std::move(recorder);

    Logger::info(
        "Recorder added: " + id
    );

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
        Logger::info(
            "Starting recorder: " + entry.first
        );

        if (!entry.second->start())
        {
            Logger::error(
                "Failed to start recorder: " + entry.first
            );

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
        Logger::info(
            "Stopping recorder: " + entry.first
        );

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

bool RecordingManager::processRecorderEvents()
{
    bool healthy = true;

    for (auto& entry : recorders_)
    {
        const std::string& id = entry.first;
        Recorder* recorder = entry.second.get();

        if (recorder == nullptr)
        {
            Logger::error(
                "[RECORDING] Recorder is null: " + id
            );

            healthy = false;
            continue;
        }

        if (!recorder->processEvents())
        {
            Logger::error(
                "[RECORDING] Recorder event processing failed: " +
                id
            );

            healthy = false;
        }
    }

    return healthy;
}

bool RecordingManager::enforceStorageLimit()
{
    if (storage_manager_ == nullptr)
    {
        Logger::error(
            "Cannot enforce storage limit: " +
            std::string("StorageManager is not available.")
        );

        return false;
    }

    Logger::info(
        "Checking storage limit..."
    );

    while (storage_manager_->isStorageLimitReached())
    {
        Logger::warn(
            "Storage limit reached."
        );

        Logger::info(
            "Current storage usage: " +
            std::to_string(
                storage_manager_->getUsagePercent()
            ) +
            "%"
        );

        if (!storage_manager_->deleteOldestSegment())
        {
            Logger::error(
                "Unable to free storage."
            );

            return false;
        }
    }

    Logger::info(
        "Storage usage is within configured limit: " +
        std::to_string(
            storage_manager_->getUsagePercent()
        ) +
        "%"
    );

    return true;
}

bool RecordingManager::monitorStorage()
{
    if (storage_manager_ == nullptr)
    {
        Logger::error(
            "Cannot monitor storage: " +
            std::string("StorageManager is not available.")
        );

        return false;
    }

    const double usage =
        storage_manager_->getUsagePercent();

    Logger::info(
        "Storage monitor: " +
        std::to_string(usage) +
        "%"
    );

    if (storage_manager_->isStorageLimitReached())
    {
        Logger::warn(
            "Storage limit reached. "
            "Enforcing storage policy..."
        );

        return enforceStorageLimit();
    }

    return true;
}

bool RecordingManager::recoverRecorder(
    const std::string& id)
{
    auto it = recorders_.find(id);

    if (it == recorders_.end())
    {
        Logger::error(
            "[RECORDING] Recorder not found: " + id
        );

        return false;
    }

    Recorder* recorder = it->second.get();

    if (recorder == nullptr)
    {
        Logger::error(
            "[RECORDING] Recorder is null: " + id
        );

        return false;
    }

    Logger::info(
        "[RECORDING] Recovering recorder: " + id
    );

    // ------------------------------------------------------------
    // Stop the existing recorder if it is still running.
    // ------------------------------------------------------------

    if (recorder->isRunning())
    {
        if (!recorder->stop())
        {
            Logger::error(
                "[RECORDING] Failed to stop recorder during recovery: " +
                id
            );

            return false;
        }
    }
    else
    {
        Logger::info(
            "[RECORDING] Recorder is already stopped: " + id
        );
    }

    // ------------------------------------------------------------
    // Start a fresh recording pipeline.
    // ------------------------------------------------------------

    if (!recorder->start())
    {
        Logger::error(
            "[RECORDING] Failed to restart recorder: " + id
        );

        return false;
    }

    Logger::info(
        "[RECORDING] Recorder recovery successful: " + id
    );

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

        if (recorder == nullptr ||
            (!recorder->isRunning() && !recorder->isStopping()))
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
        Logger::error(
            "[RECORDING TEST] Recorder not found: " + id
        );

        return false;
    }

    Recorder* recorder = it->second.get();

    if (recorder == nullptr)
    {
        Logger::error(
            "[RECORDING TEST] Recorder is null: " + id
        );

        return false;
    }

    return recorder->simulateFailure();
}