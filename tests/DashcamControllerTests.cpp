#include "TestCases.hpp"
#include "TestSupport.hpp"

#include "application/DashcamController.hpp"

#include <filesystem>

namespace bike_dashcam::tests {

bool runDashcamControllerTests() {
    TestContext context{"DashcamControllerTests"};

    const std::filesystem::path test_directory = makeUniqueTestDirectory("controller");
    const std::filesystem::path recordings_directory = test_directory / "recordings";
    const std::filesystem::path logs_directory = test_directory / "logs";
    const std::filesystem::path config_path = test_directory / "config.yaml";

    std::ostringstream config_stream;
    config_stream
        << "application:\n"
        << "  name: BIKE DASHCAM POC-1\n"
        << "  mode: Development\n"
        << "  recordings_directory: " << quotedPath(recordings_directory) << '\n'
        << "  logs_directory: " << quotedPath(logs_directory) << '\n'
        << '\n'
        << "cameras:\n"
        << "  expected_camera_count: 2\n"
        << "  target_width: 1920\n"
        << "  target_height: 1080\n"
        << "  target_fps: 30\n"
        << '\n'
        << "recording:\n"
        << "  codec: H264\n"
        << "  segment_duration_seconds: 300\n"
        << "  continuous: true\n"
        << '\n'
        << "logging:\n"
        << "  level: INFO\n"
        << "  console_enabled: true\n";

    writeTextFile(config_path, config_stream.str());

    application::DashcamController controller;
    context.expect(controller.initialize(config_path), "controller should initialize from a valid config");

    const auto& report = controller.startupReport();
    context.expect(report.ready, "startup report should indicate readiness");
    context.expect(report.checks.size() == 3, "startup report should contain three checks");
    context.expect(std::filesystem::exists(recordings_directory), "recordings directory should be created");
    context.expect(std::filesystem::exists(logs_directory), "logs directory should be created");

    std::filesystem::remove_all(test_directory);
    return context.failureCount() == 0;
}

}  // namespace bike_dashcam::tests
