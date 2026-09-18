#pragma once

#include "../camera/CameraManager.hpp"
#include "../recording/RecordingManager.hpp"
#include "../system/SystemMonitor.hpp"
#include "../storage/StorageManager.hpp"
#include "../events/EventManager.hpp"

class Watchdog
{
public:
    Watchdog(
        CameraManager* camera_manager,
        RecordingManager* recording_manager,
        StorageManager* storage_manager,
        SystemMonitor* system_monitor,
        EventManager* event_manager);

    ~Watchdog() = default;

    bool checkHealth();

private:
    CameraManager* camera_manager_;
    RecordingManager* recording_manager_;
    StorageManager* storage_manager_;
    SystemMonitor* system_monitor_;
    EventManager* event_manager_;
};