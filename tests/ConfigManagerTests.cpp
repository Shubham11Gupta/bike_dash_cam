#include "TestCases.hpp"
#include "TestSupport.hpp"

#include "configuration/ConfigManager.hpp"

#include <filesystem>

namespace bike_dashcam::tests {

bool runConfigManagerTests() {
    TestContext context{"ConfigManagerTests"};

    const std::filesystem::path test_directory = makeUniqueTestDirectory("config");
    const std::filesystem::path valid_config_path = test_directory / "valid.yaml";
    const std::filesystem::path invalid_config_path = test_directory / "invalid.yaml";

    writeTextFile(
        valid_config_path,
        "application:\n"
        "  name: Bike Dashcam Test\n"
        "  mode: Development\n"
        "  recordings_directory: recordings\n"
        "  logs_directory: logs\n"
        "\n"
        "cameras:\n"
        "  expected_camera_count: 2\n"
        "  target_width: 1280\n"
        "  target_height: 720\n"
        "  target_fps: 24\n"
        "\n"
        "recording:\n"
        "  codec: H264\n"
        "  segment_duration_seconds: 120\n"
        "  continuous: true\n"
        "\n"
        "logging:\n"
        "  level: DEBUG\n"
        "  console_enabled: true\n");

    configuration::ConfigManager valid_manager;
    context.expect(valid_manager.loadFromFile(valid_config_path), "valid config should load");

    const auto& valid_config = valid_manager.config();
    context.expect(valid_config.application.name == "Bike Dashcam Test", "application name should match");
    context.expect(valid_config.cameras.target_width == 1280, "camera width should match");
    context.expect(valid_config.cameras.target_height == 720, "camera height should match");
    context.expect(valid_config.cameras.target_fps == 24, "camera fps should match");
    context.expect(valid_config.recording.segment_duration_seconds == 120, "segment duration should match");
    context.expect(valid_config.logging.level == "DEBUG", "logging level should match");

    writeTextFile(
        invalid_config_path,
        "application:\n"
        "  name: Broken Config\n"
        "\n"
        "recording:\n"
        "  segment_duration_seconds: 0\n"
        "\n"
        "logging:\n"
        "  level: VERBOSE\n");

    configuration::ConfigManager invalid_manager;
    context.expect(!invalid_manager.loadFromFile(invalid_config_path), "invalid config should fail");
    context.expect(!invalid_manager.errors().empty(), "invalid config should report errors");

    std::filesystem::remove_all(test_directory);
    return context.failureCount() == 0;
}

}  // namespace bike_dashcam::tests
