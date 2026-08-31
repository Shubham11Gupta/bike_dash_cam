#include "camera/CameraManager.hpp"

#include <exception>

namespace bike_dashcam::camera {

CameraManager::CameraManager(logging::Logger& logger) : logger_(logger) {
}

void CameraManager::registerBackend(std::shared_ptr<ICameraBackend> backend) {
    if (backend) {
        backends_.push_back(std::move(backend));
    }
}

bool CameraManager::initialize(std::string& error_message) {
    discovered_cameras_.clear();

    try {
        for (const auto& backend : backends_) {
            if (!backend) {
                continue;
            }

            auto discovered = backend->discover();
            discovered_cameras_.insert(discovered_cameras_.end(), discovered.begin(), discovered.end());
        }

        logger_.log(
            logging::LogLevel::Debug,
            "CameraManager",
            "Initialized camera manager with " + std::to_string(backends_.size()) + " backend(s).");
        return true;
    } catch (const std::exception& exception) {
        error_message = exception.what();
        return false;
    }
}

std::size_t CameraManager::backendCount() const {
    return backends_.size();
}

std::size_t CameraManager::discoveredCameraCount() const {
    return discovered_cameras_.size();
}

const std::vector<CameraDescriptor>& CameraManager::discoveredCameras() const {
    return discovered_cameras_;
}

}  // namespace bike_dashcam::camera
