#pragma once

#include <gst/gst.h>

#include "../configuration/ConfigManager.hpp"

#include "../platform/EncoderBackend.hpp"

#include "../camera/Camera.hpp"

class Recorder
{
public:
    Recorder(
        const RecordingConfig& config,
        Camera* camera,
        EncoderBackend* encoder_backend
    );

    ~Recorder();

    bool start();
    bool wait();
    void stop();

private:
    RecordingConfig config_;
    Camera* camera_;
    EncoderBackend* encoder_backend_;

    GstElement* pipeline_;
    GstBus* bus_;
};