#pragma once

#include <memory>
#include <string>
#include <unordered_map>

#include "Recorder.hpp"

class RecordingManager
{
public:
    RecordingManager() = default;
    ~RecordingManager() = default;

    bool addRecorder(
        const std::string& id,
        std::unique_ptr<Recorder> recorder);

    Recorder* getRecorder(const std::string& id) const;

    bool startAll();
    bool stopAll();

private:
    std::unordered_map<std::string, std::unique_ptr<Recorder>> recorders_;
};