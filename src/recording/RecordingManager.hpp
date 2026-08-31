#pragma once

#include "logging/Logger.hpp"
#include "recording/IRecorder.hpp"

#include <memory>
#include <string>
#include <vector>

namespace bike_dashcam::recording {

class RecordingManager {
public:
    explicit RecordingManager(logging::Logger& logger);

    void registerRecorder(std::shared_ptr<IRecorder> recorder);
    bool initialize(const RecordingProfile& profile, std::string& error_message);

    std::size_t recorderCount() const;
    const RecordingProfile& profile() const;

private:
    logging::Logger& logger_;
    RecordingProfile profile_{};
    std::vector<std::shared_ptr<IRecorder>> recorders_;
};

}  // namespace bike_dashcam::recording
