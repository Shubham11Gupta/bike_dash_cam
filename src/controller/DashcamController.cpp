#include "DashcamController.hpp"

#include <chrono>
#include <iostream>
#include <memory>
#include <thread>

#include "../camera/SimulatedCamera.hpp"
#include "../recording/Recorder.hpp"

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
    std::cout
        << "[CONTROLLER] Initializing dashcam..."
        << std::endl;

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

    std::cout
        << "[CONTROLLER] Initialization complete."
        << std::endl;

    return true;
}

bool DashcamController::initializeConfiguration()
{
    std::cout
        << "[CONTROLLER] Loading configuration..."
        << std::endl;

    if (!config_manager_.load("config/config.yaml"))
    {
        std::cerr
            << "[CONTROLLER] Failed to load configuration."
            << std::endl;

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

    std::cout
        << "Front camera: "
        << front_camera_config.resolution
        << " @ "
        << front_camera_config.fps
        << " FPS"
        << std::endl;

    std::cout
        << "Rear camera: "
        << rear_camera_config.resolution
        << " @ "
        << rear_camera_config.fps
        << " FPS"
        << std::endl;

    std::cout
        << "Codec: "
        << recording_config.codec
        << std::endl;

    std::cout
        << "Segment duration: "
        << recording_config.segment_duration
        << " seconds"
        << std::endl;

    std::cout
        << "Storage limit: "
        << storage_config.max_usage_percent
        << "%"
        << std::endl;

    std::cout
        << "Recording path: "
        << storage_config.recording_path
        << std::endl;

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
        std::cerr
            << "[CONTROLLER] Storage initialization failed."
            << std::endl;

        return false;
    }

    std::cout
        << "Storage total: "
        << storage_manager_->getTotalSpace()
        << " bytes"
        << std::endl;

    std::cout
        << "Storage available: "
        << storage_manager_->getAvailableSpace()
        << " bytes"
        << std::endl;

    std::cout
        << "Storage used: "
        << storage_manager_->getUsedSpace()
        << " bytes"
        << std::endl;

    std::cout
        << "Storage usage: "
        << storage_manager_->getUsagePercent()
        << "%"
        << std::endl;

    std::cout
        << "Storage writable: "
        << (storage_manager_->isWritable()
            ? "YES"
            : "NO")
        << std::endl;

    if (!storage_manager_->enforceStorageLimit())
    {
        std::cerr
            << "[CONTROLLER] Storage limit enforcement failed."
            << std::endl;

        return false;
    }

    return true;
}

bool DashcamController::initializeSystemMonitor()
{
    system_monitor_ =
        new SystemMonitor();

    std::cout
        << "System monitor initialized."
        << std::endl;

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
        std::cerr
            << "[CONTROLLER] Failed to add front camera."
            << std::endl;

        return false;
    }

    if (!camera_manager_->addCamera(
            "rear",
            std::make_unique<SimulatedCamera>(
                "C:/Users/shubh/OneDrive/Desktop/work/ideation/Bike Dashcam/Videos/rear_sample.mp4"
            )))
    {
        std::cerr
            << "[CONTROLLER] Failed to add rear camera."
            << std::endl;

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
        std::cerr
            << "[CONTROLLER] Failed to retrieve cameras."
            << std::endl;

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
        std::cerr
            << "[CONTROLLER] Failed to initialize front segment manager."
            << std::endl;

        return false;
    }

    if (!rear_segment_manager_->initialize())
    {
        std::cerr
            << "[CONTROLLER] Failed to initialize rear segment manager."
            << std::endl;

        return false;
    }

    // ------------------------------------------------------------
    // Camera sources
    // ------------------------------------------------------------

    std::cout
        << "Front camera source: "
        << front_camera->getPipelineSource()
        << std::endl;

    std::cout
        << "Rear camera source: "
        << rear_camera->getPipelineSource()
        << std::endl;

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
        std::cerr
            << "[CONTROLLER] Failed to add front recorder."
            << std::endl;

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
        std::cerr
            << "[CONTROLLER] Failed to add rear recorder."
            << std::endl;

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
        std::cerr
            << "[CONTROLLER] Failed to start cameras."
            << std::endl;

        return false;
    }

    if (!camera_manager_->areAllHealthy())
    {
        std::cerr
            << "[CONTROLLER] Camera health check failed."
            << std::endl;

        return false;
    }

    if (!recording_manager_->startAll())
    {
        std::cerr
            << "[CONTROLLER] Failed to start recordings."
            << std::endl;

        return false;
    }

    std::cout
        << "Both recordings started."
        << std::endl;

    system_started_ = true;

    return true;
}

void DashcamController::simulateCameraFailure()
{
    std::cout
        << "[TEST] Simulating rear camera failure..."
        << std::endl;

    Camera* rear_camera =
        camera_manager_->getCamera("rear");

    SimulatedCamera* rear_simulated_camera =
        dynamic_cast<SimulatedCamera*>(rear_camera);

    if (rear_simulated_camera == nullptr)
    {
        std::cerr
            << "[TEST] Failed to access simulated rear camera."
            << std::endl;

        return;
    }

    rear_simulated_camera->simulateFailure();

    std::cout
        << "[TEST] Rear camera failure simulated."
        << std::endl;

    std::cout
        << "[TEST] Rear camera healthy: "
        << (rear_simulated_camera->isHealthy()
            ? "YES"
            : "NO")
        << std::endl;
}

int DashcamController::run()
{
    if (!initialized_)
    {
        std::cerr
            << "[CONTROLLER] Cannot run before initialization."
            << std::endl;

        return 1;
    }

    if (!startSystem())
    {
        stopSystem();
        return 1;
    }

    std::cout
        << "Running watchdog health check..."
        << std::endl;

    if (!watchdog_->checkHealth())
    {
        std::cerr
            << "Watchdog detected a health issue."
            << std::endl;
    }
    else
    {
        std::cout
            << "Watchdog health check passed."
            << std::endl;
    }

    // ------------------------------------------------------------
    // Temporary POC test
    // ------------------------------------------------------------

    std::cout
        << "[TEST] Waiting before simulating camera failure..."
        << std::endl;

    std::this_thread::sleep_for(
        std::chrono::seconds(10)
    );

    simulateCameraFailure();

    simulateStorageLimit();

    simulateStorageDeletionFailure();

    // ------------------------------------------------------------
    // Recording loop
    // ------------------------------------------------------------

    std::cout
        << "Recording loop started."
        << std::endl;

    running_ = true;

    while (running_)
    {
        if (shutdown_requested_)
        {
            std::cout
                << "[SYSTEM] Shutdown requested by user."
                << std::endl;

            running_ = false;
            break;
        }

        if (!watchdog_->checkHealth())
        {
            std::cerr
                << "[WATCHDOG] Health check failed."
                << std::endl;

            event_manager_.publish(
                EventType::APPLICATION_ERROR,
                "watchdog",
                "Recording stopped because a health check failed."
            );

            running_ = false;
            break;
        }

        std::cout
            << "[Monitor] CPU: "
            << system_monitor_->getCpuUsage()
            << "% | Memory: "
            << system_monitor_->getMemoryUsage()
            << "%"
            << std::endl;

        if (!recording_manager_->enforceStorageLimit())
        {
            std::cerr
                << "[Monitor] Storage limit enforcement failed."
                << std::endl;
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

    std::cout
        << "[CONTROLLER] stopSystem() started."
        << std::endl;

    if (recording_manager_ != nullptr)
    {
        std::cout
            << "[CONTROLLER] Stopping recordings..."
            << std::endl;

        recording_manager_->stopAll();

        std::cout
            << "[CONTROLLER] Recordings stopped."
            << std::endl;
    }

    if (camera_manager_ != nullptr)
    {
        std::cout
            << "[CONTROLLER] Stopping cameras..."
            << std::endl;

        camera_manager_->stopAll();

        std::cout
            << "[CONTROLLER] Cameras stopped."
            << std::endl;
    }

    system_started_ = false;
    running_ = false;

    std::cout
        << "[CONTROLLER] stopSystem() completed."
        << std::endl;
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
        std::cerr
            << "[TEST] Storage manager is unavailable."
            << std::endl;

        return;
    }

    if (!storage_manager_->simulateStorageLimit())
    {
        std::cerr
            << "[TEST] Failed to simulate storage limit."
            << std::endl;

        return;
    }

    std::cout
        << "[TEST] Storage limit simulated successfully."
        << std::endl;
}

void DashcamController::simulateStorageDeletionFailure()
{
    std::cout
        << "[TEST] Simulating storage deletion failure..."
        << std::endl;

    if (storage_manager_ == nullptr)
    {
        std::cerr
            << "[TEST] Storage manager unavailable."
            << std::endl;

        return;
    }

    if (!storage_manager_->simulateStorageDeletionFailure())
    {
        std::cerr
            << "[TEST] Failed to simulate storage deletion failure."
            << std::endl;

        return;
    }

    std::cout
        << "[TEST] Storage deletion failure simulated successfully."
        << std::endl;
}