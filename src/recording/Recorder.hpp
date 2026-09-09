#pragma once

#include <gst/gst.h>

#include "../configuration/ConfigManager.hpp"

class Recorder
{
public:
    explicit Recorder(const RecordingConfig& config);
    ~Recorder();

    bool start();
    bool wait();
    void stop();

private:
    RecordingConfig config_;

    GstElement* pipeline_;
    GstBus* bus_;
};