#include "recording/RecordingManager.hpp"

namespace bike_dashcam::recording {
namespace {

bool isProfileValid(const RecordingProfile& profile) {
    return profile.width > 0 && profile.height > 0 && profile.fps > 0 &&
           !profile.codec.empty() && profile.segment_duration_seconds > 0;
}

}  // namespace

RecordingManager::RecordingManager(logging::Logger& logger) : logger_(logger) {
}

void RecordingManager::registerRecorder(std::shared_ptr<IRecorder> recorder) {
    if (recorder) {
        recorders_.push_back(std::move(recorder));
    }
}

bool RecordingManager::initialize(const RecordingProfile& profile, std::string& error_message) {
    if (!isProfileValid(profile)) {
        error_message = "Recording profile is invalid.";
        return false;
    }

    profile_ = profile;

    for (const auto& recorder : recorders_) {
        if (!recorder) {
            continue;
        }

        if (!recorder->prepare(profile_, error_message)) {
            return false;
        }
    }

    logger_.log(
        logging::LogLevel::Debug,
        "RecordingManager",
        "Validated recording profile for Milestone 1 foundation.");
    return true;
}

std::size_t RecordingManager::recorderCount() const {
    return recorders_.size();
}

const RecordingProfile& RecordingManager::profile() const {
    return profile_;
}

}  // namespace bike_dashcam::recording
