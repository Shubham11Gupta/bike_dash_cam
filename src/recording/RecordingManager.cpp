#include "RecordingManager.hpp"

#include <iostream>

bool RecordingManager::addRecorder(
    const std::string& id,
    std::unique_ptr<Recorder> recorder)
{
    if (id.empty())
    {
        std::cerr << "Recorder ID cannot be empty."
                  << std::endl;

        return false;
    }

    if (!recorder)
    {
        std::cerr << "Cannot add null recorder."
                  << std::endl;

        return false;
    }

    if (recorders_.find(id) != recorders_.end())
    {
        std::cerr << "Recorder already exists: "
                  << id
                  << std::endl;

        return false;
    }

    recorders_[id] = std::move(recorder);

    std::cout << "Recorder added: "
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
        std::cout << "Starting recorder: "
                  << entry.first
                  << std::endl;

        if (!entry.second->start())
        {
            std::cerr << "Failed to start recorder: "
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
        std::cout << "Stopping recorder: "
                  << entry.first
                  << std::endl;

        entry.second->stop();
    }

    return success;
}