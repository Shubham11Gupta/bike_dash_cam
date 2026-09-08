#include <iostream>

#include "Recorder.hpp"

Recorder::Recorder()
    : pipeline_(nullptr),
      bus_(nullptr)
{
}

Recorder::~Recorder()
{
    stop();
}

bool Recorder::start()
{
    const char* pipeline_description =
        "filesrc location=\"C:/Users/shubh/OneDrive/Desktop/work/ideation/Bike Dashcam/Videos/sample-30s.mp4\" "
        "! decodebin "
        "! videoconvert "
        "! x264enc "
        "! h264parse "
        "! mp4mux "
        "! filesink location=\"test_cpp_recording.mp4\"";

    GError* error = nullptr;

    pipeline_ = gst_parse_launch(
        pipeline_description,
        &error
    );

    if (pipeline_ == nullptr)
    {
        std::cerr << "Failed to create GStreamer pipeline.";

        if (error != nullptr)
        {
            std::cerr << " Error: "
                      << error->message
                      << std::endl;

            g_error_free(error);
        }

        return false;
    }

    bus_ = gst_element_get_bus(pipeline_);

    GstStateChangeReturn result =
        gst_element_set_state(
            pipeline_,
            GST_STATE_PLAYING
        );

    if (result == GST_STATE_CHANGE_FAILURE)
    {
        std::cerr << "Failed to start GStreamer pipeline."
                  << std::endl;

        stop();

        return false;
    }

    std::cout << "Recording started."
              << std::endl;

    return true;
}

bool Recorder::wait()
{
    if (bus_ == nullptr)
    {
        std::cerr << "Recorder bus is not available."
                  << std::endl;

        return false;
    }

    GstMessage* message =
        gst_bus_timed_pop_filtered(
            bus_,
            GST_CLOCK_TIME_NONE,
            static_cast<GstMessageType>(
                GST_MESSAGE_ERROR |
                GST_MESSAGE_EOS
            )
        );

    if (message == nullptr)
    {
        return false;
    }

    bool success = false;

    switch (GST_MESSAGE_TYPE(message))
    {
        case GST_MESSAGE_EOS:
            std::cout << "Recording completed."
                      << std::endl;

            success = true;
            break;

        case GST_MESSAGE_ERROR:
        {
            GError* error = nullptr;
            gchar* debug_info = nullptr;

            gst_message_parse_error(
                message,
                &error,
                &debug_info
            );

            std::cerr << "GStreamer error: "
                      << error->message
                      << std::endl;

            if (debug_info != nullptr)
            {
                std::cerr << "Debug information: "
                          << debug_info
                          << std::endl;
            }

            g_error_free(error);
            g_free(debug_info);

            break;
        }

        default:
            break;
    }

    gst_message_unref(message);

    return success;
}

void Recorder::stop()
{
    if (pipeline_ != nullptr)
    {
        gst_element_set_state(
            pipeline_,
            GST_STATE_NULL
        );
    }

    if (bus_ != nullptr)
    {
        gst_object_unref(bus_);
        bus_ = nullptr;
    }

    if (pipeline_ != nullptr)
    {
        gst_object_unref(pipeline_);
        pipeline_ = nullptr;
    }
}