#include <iostream>

#include <gst/gst.h>

#include "src/recording/Recorder.hpp"

#include "src/configuration/ConfigManager.hpp"

#include "src/platform/SoftwareEncoderBackend.hpp"

#include "src/camera/SimulatedCamera.hpp"

#include <memory>

#include "src/camera/CameraManager.hpp"

int main()
{
    gst_init(nullptr, nullptr);

    std::cout << "Bike Dashcam POC-1" << std::endl;

    std::cout << "GStreamer version: "
              << gst_version_string()
              << std::endl;

    ConfigManager config_manager;

    if (!config_manager.load("config/config.yaml"))
    {
        std::cerr << "Failed to load configuration."
                << std::endl;

        gst_deinit();
        return 1;
    }

    const auto& front_camera =
        config_manager.getFrontCameraConfig();

    const auto& rear_camera =
        config_manager.getRearCameraConfig();

    const auto& recording =
        config_manager.getRecordingConfig();

    const auto& storage =
        config_manager.getStorageConfig();

    std::cout << "Front camera: "
            << front_camera.resolution
            << " @ "
            << front_camera.fps
            << " FPS"
            << std::endl;

    std::cout << "Rear camera: "
            << rear_camera.resolution
            << " @ "
            << rear_camera.fps
            << " FPS"
            << std::endl;

    std::cout << "Codec: "
            << recording.codec
            << std::endl;

    std::cout << "Segment duration: "
            << recording.segment_duration
            << " seconds"
            << std::endl;

    std::cout << "Storage limit: "
            << storage.max_usage_percent
            << "%"
            << std::endl;

    std::cout << "Recording path: "
            << storage.recording_path
            << std::endl;

    SoftwareEncoderBackend encoder_backend;

    SimulatedCamera camera(
        "C:/Users/shubh/OneDrive/Desktop/work/ideation/Bike Dashcam/Videos/sample-30s.mp4"
    );

    if (!camera.start())
    {
        std::cerr << "Failed to start simulated camera."
                << std::endl;

        gst_deinit();
        return 1;
    }

    std::cout << "Camera source: "
            << camera.getPipelineSource()
            << std::endl;

    std::cout << "Camera healthy: "
            << (camera.isHealthy() ? "YES" : "NO")
            << std::endl;

    //camera.stop();

    // CameraManager Testing
    CameraManager camera_manager;

    camera_manager.addCamera(
        "front",
        std::make_unique<SimulatedCamera>(
            "C:/Users/shubh/OneDrive/Desktop/work/ideation/Bike Dashcam/Videos/sample-30s.mp4"
        )
    );

    camera_manager.addCamera(
        "rear",
        std::make_unique<SimulatedCamera>(
            "C:/Users/shubh/OneDrive/Desktop/work/ideation/Bike Dashcam/Videos/sample-20s.mp4"
        )
    );

    camera_manager.startAll();

    std::cout << "All cameras healthy: "
            << (camera_manager.areAllHealthy() ? "YES" : "NO")
            << std::endl;

    Camera* front_camera_object = camera_manager.getCamera("front");

    if (front_camera_object)
    {
        std::cout << "Front camera source: "
                << front_camera_object->getPipelineSource()
                << std::endl;
    }

    camera_manager.stopAll();
// End of CameraManager Testing

    Recorder recorder(
        config_manager.getRecordingConfig(),
        &camera,
        &encoder_backend
    );

    if (!recorder.start())
    {
        std::cerr << "Failed to start recorder."
                  << std::endl;

        gst_deinit();

        return 1;
    }

    recorder.wait();

    recorder.stop();

    gst_deinit();

    std::cout << "Application finished."
              << std::endl;

    return 0;
}