#include "DashcamController.hpp"

#include <chrono>
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
    Logger::info("[CONTROLLER] Initializing cameras.");

    camera_manager_ =
        new CameraManager(&event_manager_);

    // Retrieve camera configurations.
    const CameraConfig& front_config =
        config_manager_.getFrontCameraConfig();

    const CameraConfig& rear_config =
        config_manager_.getRearCameraConfig();

    // Initialize front camera if enabled.
    if (front_config.enabled)
    {
        if (!camera_manager_->addCamera(
                "front",
                std::make_unique<SimulatedCamera>(
                    "/mnt/bdm/Videos/front_sample.mp4")))
        {
            Logger::error(
                "[CONTROLLER] Failed to add front camera.");

            return false;
        }

        Logger::info(
            "[CONTROLLER] Front camera initialized.");
    }
    else
    {
        Logger::info(
            "[CONTROLLER] Front camera disabled by configuration.");
    }

    // Initialize rear camera if enabled.
    if (rear_config.enabled)
    {
        if (!camera_manager_->addCamera(
                "rear",
                std::make_unique<SimulatedCamera>(
                    "/mnt/bdm/Videos/rear_sample.mp4")))
        {
            Logger::error(
                "[CONTROLLER] Failed to add rear camera.");

            return false;
        }

        Logger::info(
            "[CONTROLLER] Rear camera initialized.");
    }
    else
    {
        Logger::info(
            "[CONTROLLER] Rear camera disabled by configuration.");
    }

    // Ensure at least one camera is enabled.
    if (!front_config.enabled && !rear_config.enabled)
    {
        Logger::error(
            "[CONTROLLER] No cameras are enabled in configuration.");

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

    const auto& front_config =
        config_manager_.getFrontCameraConfig();

    const auto& rear_config =
        config_manager_.getRearCameraConfig();

    // ------------------------------------------------------------
    // Retrieve enabled cameras
    // ------------------------------------------------------------

    Camera* front_camera = nullptr;
    Camera* rear_camera = nullptr;

    if (front_config.enabled)
    {
        front_camera =
            camera_manager_->getCamera("front");

        if (front_camera == nullptr)
        {
            Logger::error(
                "[CONTROLLER] Failed to retrieve enabled front camera.");

            return false;
        }
    }

    if (rear_config.enabled)
    {
        rear_camera =
            camera_manager_->getCamera("rear");

        if (rear_camera == nullptr)
        {
            Logger::error(
                "[CONTROLLER] Failed to retrieve enabled rear camera.");

            return false;
        }
    }

    // ------------------------------------------------------------
    // Segment managers
    // ------------------------------------------------------------

    if (front_camera != nullptr)
    {
        front_segment_manager_ =
            new SegmentManager(
                storage_config.recording_path,
                "front",
                recording_config.segment_duration
            );

        if (!front_segment_manager_->initialize())
        {
            Logger::error(
                "[CONTROLLER] Failed to initialize front segment manager.");

            return false;
        }
    }

    if (rear_camera != nullptr)
    {
        rear_segment_manager_ =
            new SegmentManager(
                storage_config.recording_path,
                "rear",
                recording_config.segment_duration
            );

        if (!rear_segment_manager_->initialize())
        {
            Logger::error(
                "[CONTROLLER] Failed to initialize rear segment manager.");

            return false;
        }
    }

    // ------------------------------------------------------------
    // Camera sources
    // ------------------------------------------------------------

    if (front_camera != nullptr)
    {
        Logger::info(
            "Front camera source: " +
            front_camera->getPipelineSource()
        );
    }

    if (rear_camera != nullptr)
    {
        Logger::info(
            "Rear camera source: " +
            rear_camera->getPipelineSource()
        );
    }

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

    if (front_camera != nullptr)
    {
        if (!recording_manager_->addRecorder(
                "front",
                std::make_unique<Recorder>(
                    recording_config,
                    front_camera,
                    &encoder_backend_,
                    front_segment_manager_,
                    &event_manager_)))
        {
            Logger::error(
                "[CONTROLLER] Failed to add front recorder.");

            return false;
        }

        Logger::info(
            "[CONTROLLER] Front recorder initialized.");
    }

    // ------------------------------------------------------------
    // Rear recorder
    // ------------------------------------------------------------

    if (rear_camera != nullptr)
    {
        if (!recording_manager_->addRecorder(
                "rear",
                std::make_unique<Recorder>(
                    recording_config,
                    rear_camera,
                    &encoder_backend_,
                    rear_segment_manager_,
                    &event_manager_)))
        {
            Logger::error(
                "[CONTROLLER] Failed to add rear recorder.");

            return false;
        }

        Logger::info(
            "[CONTROLLER] Rear recorder initialized.");
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
    // Temporary POC test: Normal recording shutdown
    // ------------------------------------------------------------

    Logger::info("[TEST] Running without injected failures.");

    running_ = true;

    const auto test_start_time = std::chrono::steady_clock::now();
    const auto test_duration = std::chrono::seconds(60);

    // ------------------------------------------------------------
    // Recording loop
    // ------------------------------------------------------------

    Logger::info("[CONTROLLER] Recording loop started.");

    while (running_)
    {
        if (shutdown_requested_)
        {
            Logger::info("[SYSTEM] Shutdown requested by user.");
            running_ = false;
            break;
        }

        // End the test after the configured recording period.
        const auto elapsed_time =
            std::chrono::steady_clock::now() - test_start_time;

        if (elapsed_time >= test_duration)
        {
            Logger::info(
                "[TEST] Normal recording test period completed."
            );

            break;
        }

        // Monitor the system while recording.
        if (!watchdog_->checkHealth())
        {
            Logger::error(
                "[WATCHDOG] Health check failed during normal recording."
            );

            event_manager_.publish(
                EventType::APPLICATION_ERROR,
                "watchdog",
                "Recording stopped because a health check failed."
            );

            running_ = false;
            stopSystem();

            return 1;
        }

        Logger::info(
            "[Monitor] CPU: " +
            std::to_string(system_monitor_->getCpuUsage()) +
            "% | Memory: " +
            std::to_string(system_monitor_->getMemoryUsage()) +
            "%"
        );

        // Enforce the normal storage policy without injecting failures.
        if (!recording_manager_->enforceStorageLimit())
        {
            Logger::error(
                "[Monitor] Storage limit enforcement failed."
            );
        }

        std::this_thread::sleep_for(
            std::chrono::seconds(1)
        );
    }

    // ------------------------------------------------------------
    // Normal shutdown
    // ------------------------------------------------------------

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
    Logger::info(
        "[TEST] Simulating storage limit..."
    );

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