#include "DashcamController.hpp"

#include <chrono>
#include <iostream>
#include <memory>
#include <thread>

#include "../camera/SimulatedCamera.hpp"
#include "../recording/Recorder.hpp"
#include "../logging/Logger.hpp"

DashcamController::DashcamController()
    : initialized_(false),
      running_(false),
      system_started_(false),
      storage_manager_(nullptr),
      system_monitor_(nullptr),
      camera_manager_(nullptr),
      recording_manager_(nullptr),
      front_segment_manager_(nullptr),
      rear_segment_manager_(nullptr),
      watchdog_(nullptr),
      shutdown_requested_(false)
{
}

DashcamController::~DashcamController()
{
    stopSystem();

    delete watchdog_;
    delete recording_manager_;
    delete rear_segment_manager_;
    delete front_segment_manager_;
    delete camera_manager_;
    delete system_monitor_;
    delete storage_manager_;
}

bool DashcamController::initialize()
{
    Logger::info("[CONTROLLER] Initializing dashcam.");

    if (!initializeConfiguration())
    {
        return false;
    }

    if (!initializeStorage())
    {
        return false;
    }

    if (!initializeSystemMonitor())
    {
        return false;
    }

    if (!initializeCameras())
    {
        return false;
    }

    if (!initializeRecordings())
    {
        return false;
    }

    if (!initializeWatchdog())
    {
        return false;
    }

    initialized_ = true;

    Logger::info("[CONTROLLER] Initialization complete.");

    return true;
}

bool DashcamController::initializeConfiguration()
{
    Logger::info("[CONTROLLER] Loading configuration...");

    if (!config_manager_.load("config/config.yaml"))
    {
        Logger::error("[CONTROLLER] Failed to load configuration.");

        return false;
    }

    const auto& front_camera_config =
        config_manager_.getFrontCameraConfig();

    const auto& rear_camera_config =
        config_manager_.getRearCameraConfig();

    const auto& recording_config =
        config_manager_.getRecordingConfig();

    const auto& storage_config =
        config_manager_.getStorageConfig();

    Logger::info(
        "Front camera: " +
        front_camera_config.resolution +
        " @ " +
        std::to_string(front_camera_config.fps) +
        " FPS"
    );

    Logger::info(
        "Rear camera: " +
        rear_camera_config.resolution +
        " @ " +
        std::to_string(rear_camera_config.fps) +
        " FPS"
    );

    Logger::info(
        "Codec: " +
        recording_config.codec
    );

    Logger::info(
        "Segment duration: " +
        std::to_string(recording_config.segment_duration) +
        " seconds"
    );

    Logger::info(
        "Storage limit: " +
        std::to_string(storage_config.max_usage_percent) +
        "%"
    );

    Logger::info(
        "Recording path: " +
        storage_config.recording_path
    );

    return true;
}

bool DashcamController::initializeStorage()
{
    const auto& storage_config =
        config_manager_.getStorageConfig();

    storage_manager_ =
        new StorageManager(
            storage_config.recording_path,
            storage_config.max_usage_percent
        );

    if (!storage_manager_->initialize())
    {
        Logger::error("[CONTROLLER] Storage initialization failed.");
        return false;
    }

    Logger::info(
        "Storage total: " +
        std::to_string(storage_manager_->getTotalSpace()) +
        " bytes"
    );

    Logger::info(
        "Storage available: " +
        std::to_string(storage_manager_->getAvailableSpace()) +
        " bytes"
    );

    Logger::info(
        "Storage used: " +
        std::to_string(storage_manager_->getUsedSpace()) +
        " bytes"
    );

    Logger::info(
        "Storage usage: " +
        std::to_string(storage_manager_->getUsagePercent()) +
        "%"
    );

    Logger::info(
        "Storage writable: " +
        std::string(storage_manager_->isWritable() ? "YES" : "NO")
    );

    if (!storage_manager_->enforceStorageLimit())
    {
        Logger::error("[CONTROLLER] Storage limit enforcement failed.");
        return false;
    }

    return true;
}

bool DashcamController::initializeSystemMonitor()
{
    system_monitor_ =
        new SystemMonitor();

    Logger::info("[CONTROLLER] System monitor initialized.");

    return true;
}

bool DashcamController::initializeCameras()
{
    camera_manager_ =
        new CameraManager(&event_manager_);

    if (!camera_manager_->addCamera(
            "front",
            std::make_unique<SimulatedCamera>(
                "C:/Users/shubh/OneDrive/Desktop/work/ideation/Bike Dashcam/Videos/front_sample.mp4"
            )))
    {
        Logger::error("[CONTROLLER] Failed to add front camera.");
        return false;
    }

    if (!camera_manager_->addCamera(
            "rear",
            std::make_unique<SimulatedCamera>(
                "C:/Users/shubh/OneDrive/Desktop/work/ideation/Bike Dashcam/Videos/rear_sample.mp4"
            )))
    {
        Logger::error("[CONTROLLER] Failed to add rear camera.");
        return false;
    }

    return true;
}

bool DashcamController::initializeRecordings()
{
    const auto& recording_config =
        config_manager_.getRecordingConfig();

    const auto& storage_config =
        config_manager_.getStorageConfig();

    Camera* front_camera =
        camera_manager_->getCamera("front");

    Camera* rear_camera =
        camera_manager_->getCamera("rear");

    if (front_camera == nullptr ||
        rear_camera == nullptr)
    {
        Logger::error("[CONTROLLER] Failed to retrieve cameras.");
        return false;
    }

    // ------------------------------------------------------------
    // Segment managers
    // ------------------------------------------------------------

    front_segment_manager_ =
        new SegmentManager(
            storage_config.recording_path,
            "front",
            recording_config.segment_duration
        );

    rear_segment_manager_ =
        new SegmentManager(
            storage_config.recording_path,
            "rear",
            recording_config.segment_duration
        );

    if (!front_segment_manager_->initialize())
    {
        Logger::error("[CONTROLLER] Failed to initialize front segment manager.");
        return false;
    }

    if (!rear_segment_manager_->initialize())
    {
        Logger::error("[CONTROLLER] Failed to initialize rear segment manager.");

        return false;
    }

    // ------------------------------------------------------------
    // Camera sources
    // ------------------------------------------------------------

    Logger::info(
        "Front camera source: " +
        front_camera->getPipelineSource()
    );

    Logger::info(
        "Rear camera source: " +
        rear_camera->getPipelineSource()
    );

    // ------------------------------------------------------------
    // Recording manager
    // ------------------------------------------------------------

    recording_manager_ =
        new RecordingManager(
            storage_manager_,
            &event_manager_
        );

    // ------------------------------------------------------------
    // Front recorder
    // ------------------------------------------------------------

    if (!recording_manager_->addRecorder(
            "front",
            std::make_unique<Recorder>(
                recording_config,
                front_camera,
                &encoder_backend_,
                front_segment_manager_)))
    {
        Logger::error("[CONTROLLER] Failed to add front recorder.");
        return false;
    }

    // ------------------------------------------------------------
    // Rear recorder
    // ------------------------------------------------------------

    if (!recording_manager_->addRecorder(
            "rear",
            std::make_unique<Recorder>(
                recording_config,
                rear_camera,
                &encoder_backend_,
                rear_segment_manager_)))
    {
        Logger::error("[CONTROLLER] Failed to add rear recorder.");
        return false;
    }

    return true;
}

bool DashcamController::initializeWatchdog()
{
    watchdog_ =
        new Watchdog(
            camera_manager_,
            recording_manager_,
            storage_manager_,
            system_monitor_,
            &event_manager_
        );

    return true;
}

bool DashcamController::startSystem()
{
    if (!camera_manager_->startAll())
    {
        Logger::error("[CONTROLLER] Failed to start cameras.");
        return false;
    }

    if (!camera_manager_->areAllHealthy())
    {
        Logger::error("[CONTROLLER] Camera health check failed.");

        return false;
    }

    if (!recording_manager_->startAll())
    {
        Logger::error("[CONTROLLER] Failed to start recordings.");
        return false;
    }

    Logger::info("[CONTROLLER] Both recordings started.");

    system_started_ = true;

    return true;
}

void DashcamController::simulateCameraFailure()
{
    Logger::info("[TEST] Simulating rear camera failure...");

    Camera* rear_camera =
        camera_manager_->getCamera("rear");

    SimulatedCamera* rear_simulated_camera =
        dynamic_cast<SimulatedCamera*>(rear_camera);

    if (rear_simulated_camera == nullptr)
    {
        Logger::error("[TEST] Failed to access simulated rear camera.");

        return;
    }

    rear_simulated_camera->simulateFailure();

    Logger::info("[TEST] Rear camera failure simulated.");

    Logger::info(
        "[TEST] Rear camera healthy: " +
        std::string(rear_simulated_camera->isHealthy() ? "YES" : "NO")
    );
}

int DashcamController::run()
{
    if (!initialized_)
    {
        Logger::error("[CONTROLLER] Cannot run before initialization.");

        return 1;
    }

    if (!startSystem())
    {
        stopSystem();
        return 1;
    }

    Logger::info("[CONTROLLER] Running watchdog health check...");

    if (!watchdog_->checkHealth())
    {
        Logger::error("[CONTROLLER] Watchdog detected a health issue.");
    }
    else
    {
        Logger::info("[CONTROLLER] Watchdog health check passed.");
    }

    // ------------------------------------------------------------
    // Temporary POC test
    // ------------------------------------------------------------

    Logger::info("[TEST] Waiting before simulating camera failure...");

    std::this_thread::sleep_for(
        std::chrono::seconds(10)
    );

    simulateCameraFailure();

    simulateStorageLimit();

    simulateStorageDeletionFailure();

    // ------------------------------------------------------------
    // Recording loop
    // ------------------------------------------------------------

    Logger::info("[CONTROLLER] Recording loop started.");

    running_ = true;

    while (running_)
    {
        if (shutdown_requested_)
        {
            Logger::info("[SYSTEM] Shutdown requested by user.");

            running_ = false;
            break;
        }

        if (!watchdog_->checkHealth())
        {
            Logger::error("[WATCHDOG] Health check failed.");

            event_manager_.publish(
                EventType::APPLICATION_ERROR,
                "watchdog",
                "Recording stopped because a health check failed."
            );

            running_ = false;
            break;
        }

        Logger::info(
            "[Monitor] CPU: " +
            std::to_string(system_monitor_->getCpuUsage()) +
            "% | Memory: " +
            std::to_string(system_monitor_->getMemoryUsage()) +
            "%"
        );

        if (!recording_manager_->enforceStorageLimit())
        {
            Logger::error("[Monitor] Storage limit enforcement failed.");
        }

        std::this_thread::sleep_for(
            std::chrono::seconds(5)
        );
    }

    stopSystem();

    return 0;
}

void DashcamController::stopSystem()
{
    if (!system_started_)
    {
        return;
    }

    Logger::info("[CONTROLLER] stopSystem() started.");

    if (recording_manager_ != nullptr)
    {
        Logger::info("[CONTROLLER] Stopping recordings...");

        recording_manager_->stopAll();

        Logger::info("[CONTROLLER] Recordings stopped.");
    }

    if (camera_manager_ != nullptr)
    {
        Logger::info("[CONTROLLER] Stopping cameras...");

        camera_manager_->stopAll();

        Logger::info("[CONTROLLER] Cameras stopped.");
    }

    system_started_ = false;
    running_ = false;

    Logger::info("[CONTROLLER] stopSystem() completed.");
}

void DashcamController::requestShutdown()
{
    shutdown_requested_ = true;
}

void DashcamController::simulateStorageLimit()
{
    std::cout
        << "[TEST] Simulating storage limit..."
        << std::endl;

    if (storage_manager_ == nullptr)
    {
        Logger::error("[TEST] Storage manager is unavailable.");
        return;
    }

    if (!storage_manager_->simulateStorageLimit())
    {
        Logger::error("[TEST] Failed to simulate storage limit.");
        return;
    }

    Logger::info("[TEST] Storage limit simulated successfully.");
}

void DashcamController::simulateStorageDeletionFailure()
{
    Logger::info("[TEST] Simulating storage deletion failure...");

    if (storage_manager_ == nullptr)
    {
        Logger::error("[TEST] Storage manager unavailable.");
        return;
    }

    if (!storage_manager_->simulateStorageDeletionFailure())
    {
        Logger::error("[TEST] Failed to simulate storage deletion failure.");
        return;
    }

    Logger::info("[TEST] Storage deletion failure simulated successfully.");
}