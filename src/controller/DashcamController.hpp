#pragma once

#include <atomic>

#include "../configuration/ConfigManager.hpp"
#include "../events/EventManager.hpp"
#include "../platform/SoftwareEncoderBackend.hpp"
#include "../camera/CameraManager.hpp"
#include "../recording/RecordingManager.hpp"
#include "../recording/SegmentManager.hpp"
#include "../storage/StorageManager.hpp"
#include "../system/SystemMonitor.hpp"
#include "../watchdog/Watchdog.hpp"

class DashcamController
{
public:
    DashcamController();
    ~DashcamController();

    bool initialize();
    int run();

    void requestShutdown();

private:
    bool initializeConfiguration();
    bool initializeStorage();
    bool initializeSystemMonitor();
    bool initializeCameras();
    bool initializeRecordings();
    bool initializeWatchdog();

    bool startSystem();
    void stopSystem();

    void simulateCameraFailure();
    void simulateStorageLimit();
    void simulateStorageDeletionFailure();

private:
    // ------------------------------------------------------------
    // Application state
    // ------------------------------------------------------------

    bool initialized_;
    bool running_;
    bool system_started_;

    // ------------------------------------------------------------
    // Core services
    // ------------------------------------------------------------

    EventManager event_manager_;
    ConfigManager config_manager_;
    StorageManager* storage_manager_;
    SystemMonitor* system_monitor_;
    SoftwareEncoderBackend encoder_backend_;

    // ------------------------------------------------------------
    // Camera / recording services
    // ------------------------------------------------------------

    CameraManager* camera_manager_;
    RecordingManager* recording_manager_;

    // ------------------------------------------------------------
    // Segment managers
    // ------------------------------------------------------------

    SegmentManager* front_segment_manager_;
    SegmentManager* rear_segment_manager_;

    // ------------------------------------------------------------
    // Watchdog
    // ------------------------------------------------------------

    Watchdog* watchdog_;

    // ------------------------------------------------------------
    // Shutdown
    // ------------------------------------------------------------

    std::atomic<bool> shutdown_requested_;
};