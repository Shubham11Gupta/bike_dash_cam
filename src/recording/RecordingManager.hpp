#pragma once

#include <memory>
#include <string>
#include <unordered_map>

#include "Recorder.hpp"
#include "../storage/StorageManager.hpp"
#include "../events/EventManager.hpp"

class RecordingManager
{
public:
    explicit RecordingManager(
        StorageManager* storage_manager,
        EventManager* event_manager);

    ~RecordingManager() = default;

    bool addRecorder(
        const std::string& id,
        std::unique_ptr<Recorder> recorder);

    Recorder* getRecorder(const std::string& id) const;

    bool startAll();
    bool stopAll();

    bool enforceStorageLimit();
    bool monitorStorage();

private:
    std::unordered_map<std::string, std::unique_ptr<Recorder>> recorders_;

    StorageManager* storage_manager_;
    EventManager* event_manager_;
};