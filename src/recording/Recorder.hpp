#pragma once

#include <gst/gst.h>

class Recorder
{
public:
    Recorder();
    ~Recorder();

    bool start();
    bool wait();
    void stop();

private:
    GstElement* pipeline_;
    GstBus* bus_;
};