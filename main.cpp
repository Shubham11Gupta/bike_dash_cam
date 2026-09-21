#include <iostream>
#include <memory>
#include <chrono>
#include <thread>

#include <gst/gst.h>

#include "src/configuration/ConfigManager.hpp"
#include "src/platform/SoftwareEncoderBackend.hpp"
#include "src/camera/CameraManager.hpp"
#include "src/camera/SimulatedCamera.hpp"
#include "src/recording/Recorder.hpp"
#include "src/recording/RecordingManager.hpp"
#include "src/recording/SegmentManager.hpp"
#include "src/storage/StorageManager.hpp"
#include "src/system/SystemMonitor.hpp"
#include "src/events/EventManager.hpp"
#include "src/watchdog/Watchdog.hpp"

int main()
{
    gst_init(nullptr, nullptr);

    EventManager event_manager;

    event_manager.publish(
        EventType::SYSTEM_STARTED,
        "main",
        "Bike Dashcam POC-1 started."
    );

    std::cout << "Bike Dashcam POC-1" << std::endl;

    std::cout << "GStreamer version: "
              << gst_version_string()
              << std::endl;

    // ============================================================
    // Configuration
    // ============================================================

    ConfigManager config_manager;

    if (!config_manager.load("config/config.yaml"))
    {
        std::cerr << "Failed to load configuration."
                  << std::endl;

        gst_deinit();
        return 1;
    }

    const auto& front_camera_config =
        config_manager.getFrontCameraConfig();

    const auto& rear_camera_config =
        config_manager.getRearCameraConfig();

    const auto& recording_config =
        config_manager.getRecordingConfig();

    const auto& storage_config =
        config_manager.getStorageConfig();

    std::cout << "Front camera: "
              << front_camera_config.resolution
              << " @ "
              << front_camera_config.fps
              << " FPS"
              << std::endl;

    std::cout << "Rear camera: "
              << rear_camera_config.resolution
              << " @ "
              << rear_camera_config.fps
              << " FPS"
              << std::endl;

    std::cout << "Codec: "
              << recording_config.codec
              << std::endl;

    std::cout << "Segment duration: "
              << recording_config.segment_duration
              << " seconds"
              << std::endl;

    std::cout << "Storage limit: "
              << storage_config.max_usage_percent
              << "%"
              << std::endl;

    std::cout << "Recording path: "
              << storage_config.recording_path
              << std::endl;

    // ============================================================
    // Storage Manager
    // ============================================================

    StorageManager storage_manager(
        storage_config.recording_path,
        storage_config.max_usage_percent
    );

    if (!storage_manager.initialize())
    {
        std::cerr
            << "Storage initialization failed."
            << std::endl;

        gst_deinit();
        return 1;
    }

    std::cout
        << "Storage total: "
        << storage_manager.getTotalSpace()
        << " bytes"
        << std::endl;

    std::cout
        << "Storage available: "
        << storage_manager.getAvailableSpace()
        << " bytes"
        << std::endl;

    std::cout
        << "Storage used: "
        << storage_manager.getUsedSpace()
        << " bytes"
        << std::endl;

    std::cout
        << "Storage usage: "
        << storage_manager.getUsagePercent()
        << "%"
        << std::endl;

    std::cout
        << "Storage writable: "
        << (storage_manager.isWritable() ? "YES" : "NO")
        << std::endl;

    std::cout
        << "Storage limit reached: "
        << (storage_manager.isStorageLimitReached()
            ? "YES"
            : "NO")
        << std::endl;

    std::cout
        << "Enforcing storage limit..."
        << std::endl;

    if (!storage_manager.enforceStorageLimit())
    {
        std::cerr
            << "Storage limit enforcement failed."
            << std::endl;
    }

    // ============================================================
    // System Monitor
    // ============================================================

    SystemMonitor system_monitor;

    std::cout
        << "Testing system monitor..."
        << std::endl;

    std::cout
        << "Memory usage: "
        << system_monitor.getMemoryUsage()
        << "%"
        << std::endl;

    std::cout
        << "CPU usage sample 1: "
        << system_monitor.getCpuUsage()
        << "%"
        << std::endl;

    std::this_thread::sleep_for(
        std::chrono::seconds(2));

    std::cout
        << "CPU usage sample 2: "
        << system_monitor.getCpuUsage()
        << "%"
        << std::endl;

    // ============================================================
    // Encoder Backend
    // ============================================================

    SoftwareEncoderBackend encoder_backend;

    // ============================================================
    // Camera Manager
    // ============================================================

    CameraManager camera_manager(&event_manager);

    if (!camera_manager.addCamera(
            "front",
            std::make_unique<SimulatedCamera>(
                "C:/Users/shubh/OneDrive/Desktop/work/ideation/Bike Dashcam/Videos/front_sample.mp4"
            )))
    {
        std::cerr
            << "Failed to add front camera."
            << std::endl;

        gst_deinit();
        return 1;
    }

    if (!camera_manager.addCamera(
            "rear",
            std::make_unique<SimulatedCamera>(
                "C:/Users/shubh/OneDrive/Desktop/work/ideation/Bike Dashcam/Videos/rear_sample.mp4"
            )))
    {
        std::cerr
            << "Failed to add rear camera."
            << std::endl;

        gst_deinit();
        return 1;
    }

    // ============================================================
    // Start Cameras
    // ============================================================

    if (!camera_manager.startAll())
    {
        std::cerr
            << "Failed to start all cameras."
            << std::endl;

        camera_manager.stopAll();
        gst_deinit();
        return 1;
    }

    // ============================================================
    // Camera Health Check
    // ============================================================

    if (!camera_manager.areAllHealthy())
    {
        std::cerr
            << "Camera health check failed."
            << std::endl;

        camera_manager.stopAll();
        gst_deinit();
        return 1;
    }

    std::cout
        << "All cameras healthy: YES"
        << std::endl;

    // ============================================================
    // Get Camera Objects
    // ============================================================

    Camera* front_camera =
        camera_manager.getCamera("front");

    Camera* rear_camera =
        camera_manager.getCamera("rear");

    if (front_camera == nullptr ||
        rear_camera == nullptr)
    {
        std::cerr
            << "Failed to retrieve cameras."
            << std::endl;

        camera_manager.stopAll();
        gst_deinit();
        return 1;
    }

    // ============================================================
    // Segment Managers
    // ============================================================

    SegmentManager front_segment_manager(
        storage_config.recording_path,
        "front",
        recording_config.segment_duration
    );

    SegmentManager rear_segment_manager(
        storage_config.recording_path,
        "rear",
        recording_config.segment_duration
    );

    if (!front_segment_manager.initialize())
    {
        std::cerr
            << "Failed to initialize front segment manager."
            << std::endl;

        camera_manager.stopAll();
        gst_deinit();
        return 1;
    }

    if (!rear_segment_manager.initialize())
    {
        std::cerr
            << "Failed to initialize rear segment manager."
            << std::endl;

        camera_manager.stopAll();
        gst_deinit();
        return 1;
    }

    std::cout
        << "Front camera source: "
        << front_camera->getPipelineSource()
        << std::endl;

    std::cout
        << "Rear camera source: "
        << rear_camera->getPipelineSource()
        << std::endl;

    // ============================================================
    // Recording Manager
    // ============================================================

    RecordingManager recording_manager(
        &storage_manager,
        &event_manager
    );

    if (!recording_manager.addRecorder(
            "front",
            std::make_unique<Recorder>(
                recording_config,
                front_camera,
                &encoder_backend,
                &front_segment_manager)))
    {
        std::cerr
            << "Failed to add front recorder."
            << std::endl;

        camera_manager.stopAll();
        gst_deinit();
        return 1;
    }

    if (!recording_manager.addRecorder(
            "rear",
            std::make_unique<Recorder>(
                recording_config,
                rear_camera,
                &encoder_backend,
                &rear_segment_manager)))
    {
        std::cerr
            << "Failed to add rear recorder."
            << std::endl;

        camera_manager.stopAll();
        gst_deinit();
        return 1;
    }

    // ============================================================
    // Start Recordings
    // ============================================================

    if (!recording_manager.startAll())
    {
        std::cerr
            << "Failed to start all recorders."
            << std::endl;

        recording_manager.stopAll();
        camera_manager.stopAll();
        gst_deinit();
        return 1;
    }

    std::cout
        << "Both recordings started."
        << std::endl;

    // ============================================================
    // Watchdog
    // ============================================================

    Watchdog watchdog(
        &camera_manager,
        &recording_manager,
        &storage_manager,
        &system_monitor,
        &event_manager
    );

    std::cout
        << "Running watchdog health check..."
        << std::endl;

    if (!watchdog.checkHealth())
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

    // ============================================================
    // Camera Failure Simulation
    // ============================================================

    std::cout
        << "[TEST] Simulating rear camera failure..."
        << std::endl;

    Camera* rear_camera_base =
        camera_manager.getCamera("rear");

    SimulatedCamera* rear_simulated_camera =
        dynamic_cast<SimulatedCamera*>(rear_camera_base);

    if (rear_simulated_camera == nullptr)
    {
        std::cerr
            << "[TEST] Failed to access simulated rear camera."
            << std::endl;
    }
    else
    {
        rear_simulated_camera->simulateFailure();

        std::cout
            << "[TEST] Rear camera healthy after simulated failure: "
            << (rear_simulated_camera->isHealthy()
                ? "YES"
                : "NO")
            << std::endl;
    }

    // ============================================================
    // Recording Loop
    // ============================================================

    std::cout
        << "Recording loop started."
        << std::endl;

    bool recording_active = true;

    while (recording_active)
    {
        // --------------------------------------------------------
        // Watchdog health check
        // --------------------------------------------------------

        if (!watchdog.checkHealth())
        {
            std::cerr
                << "[WATCHDOG] Health check failed."
                << std::endl;

            event_manager.publish(
                EventType::APPLICATION_ERROR,
                "watchdog",
                "Recording stopped because a health check failed."
            );

            recording_active = false;
            break;
        }

        // --------------------------------------------------------
        // System monitoring
        // --------------------------------------------------------

        std::cout
            << "[Monitor] CPU: "
            << system_monitor.getCpuUsage()
            << "% | Memory: "
            << system_monitor.getMemoryUsage()
            << "%"
            << std::endl;

        // --------------------------------------------------------
        // Storage monitoring
        // --------------------------------------------------------

        if (!recording_manager.enforceStorageLimit())
        {
            std::cerr
                << "[Monitor] Storage limit enforcement failed."
                << std::endl;
        }

        // --------------------------------------------------------
        // POC runtime
        // --------------------------------------------------------

        std::this_thread::sleep_for(
            std::chrono::seconds(5));

        // --------------------------------------------------------
        // Temporary POC stop condition
        // --------------------------------------------------------

        // For now, run the recording loop once and stop.
        // This will later be replaced by a real shutdown
        // mechanism / controller state.

        recording_active = false;
    }

    // ============================================================
    // Stop Recorders
    // ============================================================

    std::cout
        << "Recording loop stopped."
        << std::endl;

    if (!recording_manager.stopAll())
    {
        std::cerr
            << "Failed to stop all recorders."
            << std::endl;
    }

    // ============================================================
    // Stop Cameras
    // ============================================================

    camera_manager.stopAll();

    // ============================================================
    // Shutdown
    // ============================================================

    event_manager.publish(
        EventType::SYSTEM_SHUTDOWN,
        "main",
        "Bike Dashcam POC-1 stopped."
    );

    gst_deinit();

    std::cout
        << "Application finished."
        << std::endl;

    return 0;
}