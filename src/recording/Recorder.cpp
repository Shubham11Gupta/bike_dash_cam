#include <iostream>
#include <string>
#include "Recorder.hpp"

Recorder::Recorder(
    const RecordingConfig& config,
    Camera* camera,
    EncoderBackend* encoder_backend
)
    : config_(config),
      camera_(camera),
      encoder_backend_(encoder_backend),
      pipeline_(nullptr),
      bus_(nullptr)
{
}

Recorder::~Recorder()
{
    stop();
}

bool Recorder::start()
{
    if (camera_ == nullptr)
    {
        std::cerr << "Camera is not available."
                << std::endl;

        return false;
    }
    
    std::string camera_source =
        camera_->getPipelineSource();

    GError* error = nullptr;

    std::string encoder =
        encoder_backend_->getEncoderElement(
            config_.codec
        );

    if (encoder.empty())
    {
        return false;
    }
    const long long segment_duration_ns =
    static_cast<long long>(config_.segment_duration) * 1000000000LL;

    std::string pipeline_description =
        camera_source + " "
        "! decodebin "
        "! videoconvert "
        "! " + encoder + " "
        "! h264parse "
        "! splitmuxsink "
        "location=\"test_cpp_Recording/test_cpp_segment-%02d.mp4\" "
        "max-size-time=" + std::to_string(segment_duration_ns);

    pipeline_ = gst_parse_launch(
        pipeline_description.c_str(),
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