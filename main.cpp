#include <iostream>

#include <gst/gst.h>

#include "src/recording/Recorder.hpp"

#include "src/configuration/ConfigManager.hpp"

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

    Recorder recorder(
    config_manager.getRecordingConfig());

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