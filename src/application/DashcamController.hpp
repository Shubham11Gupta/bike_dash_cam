#pragma once

#include "application/StartupReport.hpp"
#include "camera/CameraManager.hpp"
#include "configuration/ConfigManager.hpp"
#include "events/EventManager.hpp"
#include "logging/Logger.hpp"
#include "recording/RecordingManager.hpp"
#include "recording/SegmentManager.hpp"
#include "storage/StorageManager.hpp"
#include "system/SystemMonitor.hpp"
#include "system/Watchdog.hpp"

#include <filesystem>

namespace bike_dashcam::application {

class DashcamController {
public:
    DashcamController();

    bool initialize(const std::filesystem::path& config_path);
    const StartupReport& startupReport() const;

private:
    logging::Logger logger_;
    configuration::ConfigManager config_manager_;
    events::EventManager event_manager_;
    camera::CameraManager camera_manager_;
    recording::RecordingManager recording_manager_;
    recording::SegmentManager segment_manager_;
    storage::StorageManager storage_manager_;
    system::SystemMonitor system_monitor_;
    system::Watchdog watchdog_;
    StartupReport report_;
};

}  // namespace bike_dashcam::application
