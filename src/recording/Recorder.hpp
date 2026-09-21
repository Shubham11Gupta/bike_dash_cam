#pragma once

#include <gst/gst.h>

#include "../configuration/ConfigManager.hpp"
#include "../camera/Camera.hpp"
#include "../platform/EncoderBackend.hpp"
#include "SegmentManager.hpp"

class Recorder
{
public:
    Recorder(
        const RecordingConfig& config,
        Camera* camera,
        EncoderBackend* encoder_backend,
        SegmentManager* segment_manager
    );

    ~Recorder();

    bool start();
    bool wait();
    bool stop();
    bool forceStop();

private:
    RecordingConfig config_;

    Camera* camera_;
    EncoderBackend* encoder_backend_;
    SegmentManager* segment_manager_;

    GstElement* pipeline_;
    GstBus* bus_;
};