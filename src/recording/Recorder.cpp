#include <string>
#include "Recorder.hpp"

#include "../logging/Logger.hpp"

Recorder::Recorder(
    const RecordingConfig& config,
    Camera* camera,
    EncoderBackend* encoder_backend,
    SegmentManager* segment_manager,
    EventManager* event_manager)
    : config_(config),
      camera_(camera),
      encoder_backend_(encoder_backend),
      segment_manager_(segment_manager),
      event_manager_(event_manager),
      pipeline_(nullptr),
      bus_(nullptr),
      failed_(false),
      stopping_(false)
{
}

Recorder::~Recorder()
{
    if (pipeline_ != nullptr)
    {
        stop();
    }
}

gchar* Recorder::onFormatLocation(
    GstElement* splitmux,
    guint fragment_id,
    gpointer user_data)
{
    Recorder* recorder =
        static_cast<Recorder*>(user_data);

    if (recorder == nullptr ||
        recorder->segment_manager_ == nullptr)
    {
        return nullptr;
    }

    const std::string location =
        recorder->segment_manager_->getSegmentPath();

    Logger::info(
        "[RECORDER] Creating segment " +
        std::to_string(fragment_id) +
        ": " +
        location
    );

    if (recorder->event_manager_ != nullptr)
    {
        recorder->event_manager_->publish(
            EventType::SEGMENT_CREATED,
            recorder->segment_manager_->getCameraId(),
            "Segment created: " + location
        );
    }

    return g_strdup(location.c_str());
}

bool Recorder::start()
{
    if (pipeline_ != nullptr)
    {
        Logger::error(
            "Recorder is already running."
        );

        return false;
    }
    if (camera_ == nullptr)
    {
        Logger::error(
            "Camera is not available."
        );

        return false;
    }

    if (segment_manager_ == nullptr)
    {
        Logger::error(
            "Segment manager is null."
        );

        return false;
    }

    std::string camera_source =
        camera_->getPipelineSource();

    if (camera_source.empty())
    {
        Logger::error(
            "Camera pipeline source is empty."
        );

        return false;
    }

    GError* error = nullptr;

    if (encoder_backend_ == nullptr)
    {
        Logger::error(
            "Encoder backend is not available."
        );

        return false;
    }

    std::string encoder =
        encoder_backend_->getEncoderElement(
            config_.codec
        );

    if (encoder.empty())
    {
        Logger::error(
            "Encoder element is not available for codec: " +
            config_.codec
        );

        return false;
    }

    failed_ = false;
    stopping_ = false;

    const long long segment_duration_ns =
        static_cast<long long>(config_.segment_duration)
        * 1000000000LL;

    std::string pipeline_description =
        camera_source +
        " ! decodebin"
        " ! videoconvert"
        " ! " + encoder +
        " ! h264parse"
        " ! splitmuxsink name=segmenter"
        " location=\"placeholder.mp4\""
        " max-size-time=" +
        std::to_string(segment_duration_ns);

    Logger::info(
        "GStreamer pipeline: " +
        pipeline_description
    );

    pipeline_ = gst_parse_launch(
        pipeline_description.c_str(),
        &error
    );

    if (pipeline_ == nullptr)
    {
        Logger::error(
            "Failed to create GStreamer pipeline."
        );

        if (error != nullptr)
        {
            Logger::error(
                "GStreamer error: " +
                std::string(error->message)
            );

            g_error_free(error);
        }

        return false;
    }

    GstElement* splitmux =
        gst_bin_get_by_name(
            GST_BIN(pipeline_),
            "segmenter"
        );

    if (splitmux == nullptr)
    {
        Logger::error(
            "Failed to find splitmuxsink element."
        );

        forceStop();

        return false;
    }

    g_signal_connect(
        splitmux,
        "format-location",
        G_CALLBACK(Recorder::onFormatLocation),
        this
    );

    gst_object_unref(splitmux);

    bus_ = gst_element_get_bus(pipeline_);

    GstStateChangeReturn result =
        gst_element_set_state(
            pipeline_,
            GST_STATE_PLAYING
        );

    if (result == GST_STATE_CHANGE_FAILURE)
    {
        Logger::error(
            "Failed to start GStreamer pipeline."
        );

        forceStop();

        return false;
    }

    Logger::info(
        "Recording started."
    );

    return true;
}

bool Recorder::processEvents()
{
    if (pipeline_ == nullptr || bus_ == nullptr)
    {
        return false;
    }

    GstMessage* message =
        gst_bus_pop_filtered(
            bus_,
            static_cast<GstMessageType>(
                GST_MESSAGE_ERROR |
                GST_MESSAGE_EOS
            )
        );

    if (message == nullptr)
    {
        return true;
    }

    switch (GST_MESSAGE_TYPE(message))
    {
        case GST_MESSAGE_ERROR:
        {
            GError* error = nullptr;
            gchar* debug_info = nullptr;

            gst_message_parse_error(
                message,
                &error,
                &debug_info
            );

            Logger::error(
                "[RECORDER] GStreamer error: " +
                std::string(
                    error
                        ? error->message
                        : "Unknown error"
                )
            );

            failed_ = true;

            if (debug_info != nullptr)
            {
                Logger::error(
                    "[RECORDER] Debug info: " +
                    std::string(debug_info)
                );
            }

            if (error != nullptr)
            {
                g_error_free(error);
            }

            if (debug_info != nullptr)
            {
                g_free(debug_info);
            }

            gst_message_unref(message);
            return false;
        }

        case GST_MESSAGE_EOS:
        {
            Logger::info(
                "[RECORDER] GStreamer pipeline reached EOS."
            );

            gst_message_unref(message);
            return false;
        }

        default:
            gst_message_unref(message);
            return true;
    }
}

bool Recorder::wait()
{
    if (bus_ == nullptr)
    {
        Logger::error(
            "Recorder bus is not available."
        );

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
            if (stopping_)
            {
                Logger::info(
                    "Recording stopped normally."
                );

                success = true;
            }
            else
            {
                Logger::error(
                    "[RECORDER] Unexpected EOS received."
                );

                failed_ = true;
                success = false;
            }

            break;

        case GST_MESSAGE_ERROR:
        {
            failed_ = true;

            GError* error = nullptr;
            gchar* debug_info = nullptr;

            gst_message_parse_error(
                message,
                &error,
                &debug_info
            );

            Logger::error(
                "GStreamer error: " +
                std::string(
                    error
                        ? error->message
                        : "Unknown error"
                )
            );

            if (debug_info != nullptr)
            {
                Logger::error(
                    "Debug information: " +
                    std::string(debug_info)
                );
            }

            if (error != nullptr)
            {
                g_error_free(error);
            }

            if (debug_info != nullptr)
            {
                g_free(debug_info);
            }

            break;
        }

        default:
            break;
    }

    gst_message_unref(message);

    return success;
}

bool Recorder::stop()
{
    if (pipeline_ == nullptr)
    {
        Logger::error(
            "Recorder is not running."
        );

        return false;
    }

    stopping_ = true;

    bool success = true;

    Logger::info(
        "Stopping recording gracefully."
    );

    if (!gst_element_send_event(
            pipeline_,
            gst_event_new_eos()))
    {
        Logger::error(
            "[RECORDER] Failed to send EOS to recorder."
        );

        success = false;
    }
    else
    {
        GstMessage* message =
            gst_bus_timed_pop_filtered(
                bus_,
                3 * GST_SECOND,
                static_cast<GstMessageType>(
                    GST_MESSAGE_EOS |
                    GST_MESSAGE_ERROR
                ));

        if (message == nullptr)
        {
            Logger::error(
                "[RECORDER] EOS timeout while stopping."
            );

            success = false;
        }
        else
        {
            if (GST_MESSAGE_TYPE(message) == GST_MESSAGE_EOS)
            {
                Logger::info(
                    "Recording finalized."
                );
            }
            else if (
                GST_MESSAGE_TYPE(message) ==
                GST_MESSAGE_ERROR)
            {
                GError* error = nullptr;
                gchar* debug_info = nullptr;

                gst_message_parse_error(
                    message,
                    &error,
                    &debug_info
                );

                Logger::error(
                    "[RECORDER] Error while stopping recording: " +
                    std::string(
                        error
                            ? error->message
                            : "Unknown error"
                    )
                );

                if (debug_info != nullptr)
                {
                    Logger::error(
                        "[RECORDER] Debug information: " +
                        std::string(debug_info)
                    );
                }

                if (error != nullptr)
                {
                    g_error_free(error);
                }

                if (debug_info != nullptr)
                {
                    g_free(debug_info);
                }

                success = false;
            }

            gst_message_unref(message);
        }
    }

    // ------------------------------------------------------------
    // If graceful shutdown failed, force-stop the pipeline.
    // ------------------------------------------------------------

    if (!success)
    {
        Logger::error(
            "[RECORDER] Falling back to force stop."
        );

        return forceStop();
    }

    // ------------------------------------------------------------
    // Graceful shutdown succeeded.
    // ------------------------------------------------------------

    gst_element_set_state(
        pipeline_,
        GST_STATE_NULL
    );

    if (bus_ != nullptr)
    {
        gst_object_unref(bus_);
        bus_ = nullptr;
    }

    gst_object_unref(pipeline_);
    pipeline_ = nullptr;

    return true;
}

bool Recorder::isRunning() const
{
    return pipeline_ != nullptr;
}

// ------------------------------------------------------------
// Simulate Recording Failure
// ------------------------------------------------------------

bool Recorder::simulateFailure()
{
    if (pipeline_ == nullptr)
    {
        Logger::error(
            "[RECORDER TEST] Recorder is already inactive."
        );

        return false;
    }

    Logger::info(
        "[RECORDER TEST] Simulating recorder failure."
    );

    // Force the GStreamer pipeline down without performing
    // the normal graceful EOS shutdown.
    gst_element_set_state(
        pipeline_,
        GST_STATE_NULL
    );

    if (bus_ != nullptr)
    {
        gst_object_unref(bus_);
        bus_ = nullptr;
    }

    gst_object_unref(pipeline_);
    pipeline_ = nullptr;

    Logger::info(
        "[RECORDER TEST] Recorder failure simulated."
    );

    return true;
}

// ------------------------------------------------------------
// Force stop the recording without waiting for EOS.
// This may result in a corrupted segment.
// ------------------------------------------------------------

bool Recorder::forceStop()
{
    if (pipeline_ == nullptr)
    {
        Logger::error(
            "[RECORDER] Pipeline is not running."
        );

        return false;
    }

    Logger::info(
        "[RECORDER] Force stopping recording."
    );

    // Do not wait for EOS.
    // This path is intended for recovery from
    // an unhealthy or potentially blocked pipeline.

    gst_element_set_state(
        pipeline_,
        GST_STATE_NULL
    );

    if (bus_ != nullptr)
    {
        gst_object_unref(bus_);
        bus_ = nullptr;
    }

    gst_object_unref(pipeline_);
    pipeline_ = nullptr;

    Logger::info(
        "[RECORDER] Recording pipeline force stopped."
    );

    return true;
}

bool Recorder::hasFailed() const
{
    return failed_;
}

bool Recorder::isStopping() const
{
    return stopping_;
}