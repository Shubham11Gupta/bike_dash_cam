#include "application/DashcamController.hpp"

#include "events/Event.hpp"
#include "camera/CameraBackendFactory.hpp"
#include "platform/PlatformInfo.hpp"

#include <chrono>

namespace bike_dashcam::application {
namespace {

#ifndef BIKE_DASHCAM_VERSION
#define BIKE_DASHCAM_VERSION "0.1.0"
#endif

std::string joinErrors(const std::vector<std::string>& errors) {
    std::string result;

    for (std::size_t index = 0; index < errors.size(); ++index) {
        if (index > 0) {
            result += " | ";
        }

        result += errors[index];
    }

    return result;
}

recording::RecordingProfile buildRecordingProfile(const configuration::DashcamConfig& config) {
    recording::RecordingProfile profile{};
    profile.width = config.cameras.target_width;
    profile.height = config.cameras.target_height;
    profile.fps = config.cameras.target_fps;
    profile.codec = config.recording.codec;
    profile.segment_duration_seconds = config.recording.segment_duration_seconds;
    profile.continuous = config.recording.continuous;
    return profile;
}

}  // namespace

DashcamController::DashcamController()
    : DashcamController(camera::createPlatformCameraBackends()) {
}

DashcamController::DashcamController(std::vector<std::shared_ptr<camera::ICameraBackend>> camera_backends)
    : logger_(),
      camera_manager_(logger_),
      recording_manager_(logger_),
      watchdog_(std::chrono::seconds{5}) {
    for (auto& backend : camera_backends) {
        camera_manager_.registerBackend(std::move(backend));
    }
}

bool DashcamController::initialize(const std::filesystem::path& config_path) {
    report_ = StartupReport{};
    report_.version = BIKE_DASHCAM_VERSION;

    const platform::PlatformInfo platform_info = platform::detectPlatformInfo();
    report_.platform = platform_info.operating_system;

    const bool configuration_ok = config_manager_.loadFromFile(config_path);
    report_.mode = config_manager_.config().application.mode;
    report_.checks.push_back({
        "Configuration",
        configuration_ok,
        configuration_ok ? std::string{} : joinErrors(config_manager_.errors())});

    bool application_ok = false;
    bool system_ok = false;
    std::string application_detail;
    std::string system_detail;

    if (configuration_ok) {
        if (const auto log_level = logging::parseLogLevel(config_manager_.config().logging.level)) {
            logger_.setMinimumLevel(*log_level);
        }

        std::string storage_error;
        const bool storage_ready = storage_manager_.initialize(config_manager_.config(), storage_error);

        std::string system_monitor_error;
        const bool system_monitor_ready = system_monitor_.initialize(system_monitor_error);
        system_ok = storage_ready && system_monitor_ready;
        if (!storage_ready) {
            system_detail = storage_error;
        } else if (!system_monitor_ready) {
            system_detail = system_monitor_error;
        }

        const bool segment_ready = segment_manager_.setSegmentDuration(
            std::chrono::seconds{config_manager_.config().recording.segment_duration_seconds});

        std::string recording_error;
        const bool recording_ready = recording_manager_.initialize(
            buildRecordingProfile(config_manager_.config()),
            recording_error);

        std::string camera_error;
        const bool cameras_ready = camera_manager_.initialize(
            static_cast<std::size_t>(config_manager_.config().cameras.expected_camera_count),
            config_manager_.config().cameras.preferred_camera_name,
            config_manager_.config().cameras.ignored_camera_name,
            camera_error);

        std::vector<camera::CaptureStatistics> capture_statistics;
        std::string capture_error;
        const bool capture_ready = cameras_ready && camera_manager_.captureFor(
            std::chrono::seconds{config_manager_.config().cameras.capture_duration_seconds},
            capture_statistics,
            capture_error);
        for (const auto& statistics : capture_statistics) {
            logger_.log(
                logging::LogLevel::Info,
                "CameraManager",
                "Capture " + statistics.camera_name + ": " + std::to_string(statistics.frame_count) +
                    " frames, " + std::to_string(statistics.fps) + " FPS, " +
                    std::to_string(statistics.dropped_frame_count) + " dropped.");
        }

        application_ok = segment_ready && recording_ready && cameras_ready && capture_ready && watchdog_.isHealthy();
        if (!segment_ready) {
            application_detail = "Segment duration must be positive.";
        } else if (!recording_ready) {
            application_detail = recording_error;
        } else if (!cameras_ready) {
            application_detail = camera_error;
        } else if (!capture_ready) {
            application_detail = capture_error;
        } else if (!watchdog_.isHealthy()) {
            application_detail = "Watchdog reported unhealthy state during startup.";
        }

        watchdog_.noteHeartbeat();
        event_manager_.publish({
            "startup",
            events::EventSeverity::Info,
            "DashcamController",
            "Milestone 4 dual-camera capture probe completed."});
    } else {
        application_detail = "Skipped because configuration failed.";
        system_detail = "Skipped because configuration failed.";
    }

    report_.checks.push_back({"Application", application_ok, application_detail});
    report_.checks.push_back({"System", system_ok, system_detail});
    report_.ready = configuration_ok && application_ok && system_ok;

    return report_.ready;
}

const StartupReport& DashcamController::startupReport() const {
    return report_;
}

}  // namespace bike_dashcam::application
