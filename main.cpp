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

int main()
{
    gst_init(nullptr, nullptr);

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
    // Get Storage Objects
    // ============================================================

    const auto& storage =
        config_manager.getStorageConfig();

    StorageManager storage_manager(
        storage.recording_path,
        storage.max_usage_percent
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

    // ============================================================
    // Encoder Backend
    // ============================================================

    SoftwareEncoderBackend encoder_backend;

    // ============================================================
    // Camera Manager
    // ============================================================

    CameraManager camera_manager;

    if (!camera_manager.addCamera(
            "front",
            std::make_unique<SimulatedCamera>(
                "C:/Users/shubh/OneDrive/Desktop/work/ideation/Bike Dashcam/Videos/sample-30s.mp4"
            )))
    {
        std::cerr << "Failed to add front camera."
                  << std::endl;

        gst_deinit();
        return 1;
    }

    if (!camera_manager.addCamera(
            "rear",
            std::make_unique<SimulatedCamera>(
                "C:/Users/shubh/OneDrive/Desktop/work/ideation/Bike Dashcam/Videos/sample-20s.mp4"
            )))
    {
        std::cerr << "Failed to add rear camera."
                  << std::endl;

        gst_deinit();
        return 1;
    }

    // ============================================================
    // Start Cameras
    // ============================================================

    if (!camera_manager.startAll())
    {
        std::cerr << "Failed to start all cameras."
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
        std::cerr << "Camera health check failed."
                  << std::endl;

        camera_manager.stopAll();
        gst_deinit();
        return 1;
    }

    std::cout << "All cameras healthy: YES"
              << std::endl;

    // ============================================================
    // Get Camera Objects
    // ============================================================

    Camera* front_camera =
        camera_manager.getCamera("front");

    Camera* rear_camera =
        camera_manager.getCamera("rear");
    
    SegmentManager front_segment_manager(
        config_manager.getStorageConfig().recording_path,
        "front",
        config_manager.getRecordingConfig().segment_duration
    );

    SegmentManager rear_segment_manager(
        config_manager.getStorageConfig().recording_path,
        "rear",
        config_manager.getRecordingConfig().segment_duration
    );

    if (!front_segment_manager.initialize())
    {
        std::cerr << "Failed to initialize front segment manager."
                << std::endl;

        camera_manager.stopAll();
        gst_deinit();
        return 1;
    }

    if (!rear_segment_manager.initialize())
    {
        std::cerr << "Failed to initialize rear segment manager."
                << std::endl;

        camera_manager.stopAll();
        gst_deinit();
        return 1;
    }

    if (front_camera == nullptr ||
        rear_camera == nullptr)
    {
        std::cerr << "Failed to retrieve cameras."
                  << std::endl;

        camera_manager.stopAll();
        gst_deinit();
        return 1;
    }

    std::cout << "Front camera source: "
              << front_camera->getPipelineSource()
              << std::endl;

    std::cout << "Rear camera source: "
              << rear_camera->getPipelineSource()
              << std::endl;

    // ============================================================
    // Segment Manager Testing
    // ============================================================

    /*SegmentManager front_segment_manager(
        config_manager.getStorageConfig().recording_path,
        "front",
        config_manager.getRecordingConfig().segment_duration
    );

    SegmentManager rear_segment_manager(
        config_manager.getStorageConfig().recording_path,
        "rear",
        config_manager.getRecordingConfig().segment_duration
    );

    if (!front_segment_manager.initialize())
    {
        std::cerr << "Failed to initialize front segment manager."
                << std::endl;

        camera_manager.stopAll();
        gst_deinit();

        return 1;
    }

    if (!rear_segment_manager.initialize())
    {
        std::cerr << "Failed to initialize rear segment manager."
                << std::endl;

        camera_manager.stopAll();
        gst_deinit();

        return 1;
    }

    std::cout << "Front output pattern: "
            << front_segment_manager.getOutputPattern()
            << std::endl;

    std::cout << "Rear output pattern: "
            << rear_segment_manager.getOutputPattern()
            << std::endl;*/

    // ============================================================
    // End Segment Manager Testing
    // ============================================================

    // ============================================================
    // Create Recorders
    // ============================================================
    
    RecordingManager recording_manager;

    // ============================================================
    // Start Front Recording
    // ============================================================

    if (!recording_manager.addRecorder(
            "front",
            std::make_unique<Recorder>(
                config_manager.getRecordingConfig(),
                front_camera,
                &encoder_backend,
                &front_segment_manager)))
    {
        std::cerr << "Failed to add front recorder."
                << std::endl;

        camera_manager.stopAll();
        gst_deinit();

        return 1;
    }

    // ============================================================
    // Start Rear Recording
    // ============================================================

    if (!recording_manager.addRecorder(
            "rear",
            std::make_unique<Recorder>(
                config_manager.getRecordingConfig(),
                rear_camera,
                &encoder_backend,
                &rear_segment_manager)))
    {
        std::cerr << "Failed to add rear recorder."
                << std::endl;

        camera_manager.stopAll();
        gst_deinit();

        return 1;
    }

    // ============================================================
    // Start All for Recordings
    // ============================================================

    if (!recording_manager.startAll())
    {
        std::cerr << "Failed to start all recorders."
                << std::endl;

        recording_manager.stopAll();
        camera_manager.stopAll();
        gst_deinit();

        return 1;
    }

    std::cout << "Both recordings started."
            << std::endl;
    
    // ============================================================
    // Wait for Recordings
    // ============================================================

    std::this_thread::sleep_for(
        std::chrono::seconds(18));

    // ============================================================
    // Stop Recorders
    // ============================================================

    recording_manager.stopAll();

    if (storage_manager.isStorageLimitReached())
    {
        storage_manager.deleteOldestSegment();
    }

    // ============================================================
    // Stop Cameras
    // ============================================================

    camera_manager.stopAll();

    // ============================================================
    // Shutdown
    // ============================================================

    gst_deinit();

    std::cout << "Application finished."
              << std::endl;

    return 0;
}