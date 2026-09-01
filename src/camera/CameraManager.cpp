#include "camera/CameraManager.hpp"

#include <algorithm>
#include <cctype>
#include <exception>
#include <thread>

namespace {

std::string lowercase(const std::string_view value) {
    std::string result{value};
    std::transform(result.begin(), result.end(), result.begin(), [](const unsigned char character) {
        return static_cast<char>(std::tolower(character));
    });
    return result;
}

bool matchesPreferredName(const bike_dashcam::camera::CameraDescriptor& camera, const std::string_view preferred_name) {
    return preferred_name.empty() || lowercase(camera.name).find(lowercase(preferred_name)) != std::string::npos;
}
bool matchesIgnoredName(const bike_dashcam::camera::CameraDescriptor& camera, const std::string_view ignored_name) {
    return !ignored_name.empty() && lowercase(camera.name).find(lowercase(ignored_name)) != std::string::npos;
}

}  // namespace

namespace bike_dashcam::camera {

CameraManager::CameraManager(logging::Logger& logger) : logger_(logger) {
}

void CameraManager::registerBackend(std::shared_ptr<ICameraBackend> backend) {
    if (backend) {
        backends_.push_back(std::move(backend));
    }
}

bool CameraManager::initialize(
    const std::size_t required_camera_count,
    const std::string_view preferred_camera_name,
    const std::string_view ignored_camera_name,
    std::string& error_message) {
    discovered_cameras_.clear();
    initialized_cameras_.clear();

    if (required_camera_count == 0) {
        error_message = "At least one camera is required.";
        return false;
    }

    try {
        for (const auto& backend : backends_) {
            if (!backend) {
                continue;
            }

            auto discovered = backend->discover();
            discovered_cameras_.insert(discovered_cameras_.end(), discovered.begin(), discovered.end());
            for (const auto& camera : discovered) {
                logger_.log(
                    logging::LogLevel::Info,
                    "CameraManager",
                    "Discovered camera: " + camera.name + " (" + backend->backendName() + ")");
            }
        }

        for (const auto& camera : discovered_cameras_) {
            if (!camera.available || matchesIgnoredName(camera, ignored_camera_name) ||
                !matchesPreferredName(camera, preferred_camera_name)) {
                continue;
            }

            const auto backend = std::find_if(backends_.begin(), backends_.end(), [&camera](const auto& candidate) {
                return candidate && candidate->backendName() == camera.backend_name;
            });
            if (backend == backends_.end()) {
                continue;
            }

            std::string initialization_error;
            if (!(*backend)->initialize(camera, initialization_error)) {
                logger_.log(logging::LogLevel::Warning, "CameraManager", "Could not initialize " + camera.name + ": " + initialization_error);
                continue;
            }

            initialized_cameras_.push_back(camera);
            if (initialized_cameras_.size() == required_camera_count) {
                break;
            }
        }

        if (initialized_cameras_.size() != required_camera_count) {
            error_message = "Required " + std::to_string(required_camera_count) + " initialized camera(s), found " +
                std::to_string(initialized_cameras_.size()) + ".";
            return false;
        }

        logger_.log(
            logging::LogLevel::Debug,
            "CameraManager",
            "Initialized " + std::to_string(initialized_cameras_.size()) + " camera(s) using " +
                std::to_string(backends_.size()) + " backend(s).");
        return true;
    } catch (const std::exception& exception) {
        error_message = exception.what();
        return false;
    }
}

bool CameraManager::captureFor(const std::chrono::milliseconds duration, std::vector<CaptureStatistics>& statistics,
                               std::string& error_message) {
    statistics.clear();
    error_message.clear();
    if (initialized_cameras_.empty() || duration.count() <= 0) {
        error_message = "Initialized cameras and a positive capture duration are required.";
        return false;
    }

    struct Result { CaptureStatistics statistics; std::string error; bool success{false}; };
    std::vector<Result> results(initialized_cameras_.size());
    std::vector<std::thread> workers;
    workers.reserve(initialized_cameras_.size());
    for (std::size_t index = 0; index < initialized_cameras_.size(); ++index) {
        workers.emplace_back([this, duration, index, &results] {
            const auto& camera = initialized_cameras_[index];
            const auto backend = std::find_if(backends_.begin(), backends_.end(), [&camera](const auto& candidate) {
                return candidate && candidate->backendName() == camera.backend_name;
            });
            if (backend != backends_.end()) {
                results[index].success = (*backend)->captureFor(camera, duration, results[index].statistics, results[index].error);
            } else {
                results[index].error = "Owning camera backend is unavailable.";
            }
        });
    }
    for (auto& worker : workers) { worker.join(); }
    bool success = true;
    for (const auto& result : results) {
        statistics.push_back(result.statistics);
        success = success && result.success;
        if (!result.success && error_message.empty()) { error_message = result.error; }
    }
    return success;
}

std::size_t CameraManager::backendCount() const {
    return backends_.size();
}

std::size_t CameraManager::discoveredCameraCount() const {
    return discovered_cameras_.size();
}

std::size_t CameraManager::initializedCameraCount() const {
    return initialized_cameras_.size();
}

const std::vector<CameraDescriptor>& CameraManager::discoveredCameras() const {
    return discovered_cameras_;
}

const std::vector<CameraDescriptor>& CameraManager::initializedCameras() const {
    return initialized_cameras_;
}

}  // namespace bike_dashcam::camera
