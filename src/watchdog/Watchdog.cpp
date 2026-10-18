#include "Watchdog.hpp"

#include <unordered_set>

#include "../logging/Logger.hpp"

namespace
{
    constexpr double MAX_CPU_USAGE_PERCENT = 90.0;
    constexpr double MAX_MEMORY_USAGE_PERCENT = 90.0;
}

Watchdog::Watchdog(
    CameraManager* camera_manager,
    RecordingManager* recording_manager,
    StorageManager* storage_manager,
    SystemMonitor* system_monitor,
    EventManager* event_manager)
    : camera_manager_(camera_manager),
      recording_manager_(recording_manager),
      storage_manager_(storage_manager),
      system_monitor_(system_monitor),
      event_manager_(event_manager)
{
}

bool Watchdog::checkHealth()
{
    bool healthy = true;
    std::unordered_set<std::string> recovered_recorders;

    if (recording_manager_ != nullptr)
    {
        if (!recording_manager_->processRecorderEvents())
        {
            Logger::error(
                "[WATCHDOG] Recorder event processing reported a failure."
            );

            healthy = false;
        }
    }
    else
    {
        Logger::error(
            "[WATCHDOG] Recording manager is null."
        );

        healthy = false;
    }

    // ------------------------------------------------------------
    // Camera health and recovery
    // ------------------------------------------------------------

    if (camera_manager_ == nullptr)
    {
        Logger::error(
            "[WATCHDOG] CameraManager unavailable."
        );

        healthy = false;
    }
    else
    {
        auto unhealthy_cameras =
            camera_manager_->getUnhealthyCameras();

        for (const auto& camera_id : unhealthy_cameras)
        {
            Logger::error(
                "[WATCHDOG] Camera unhealthy: " +
                camera_id
            );

            if (event_manager_ != nullptr)
            {
                event_manager_->publish(
                    EventType::CAMERA_DISCONNECTED,
                    "watchdog",
                    "Camera is unhealthy: " + camera_id
                );

                event_manager_->publish(
                    EventType::CAMERA_RECOVERY_STARTED,
                    "watchdog",
                    "Attempting camera recovery: " + camera_id
                );
            }

            bool camera_recovered =
                camera_manager_->recoverCamera(camera_id);

            if (!camera_recovered)
            {
                Logger::error(
                    "[WATCHDOG] Camera recovery failed: " +
                    camera_id
                );

                if (event_manager_ != nullptr)
                {
                    event_manager_->publish(
                        EventType::CAMERA_RECOVERY_FAILED,
                        "watchdog",
                        "Camera recovery failed: " + camera_id
                    );
                }

                healthy = false;
                continue;
            }

            if (event_manager_ != nullptr)
            {
                event_manager_->publish(
                    EventType::CAMERA_RECOVERY_SUCCESS,
                    "watchdog",
                    "Camera recovered successfully: " + camera_id
                );
            }

            // --------------------------------------------------------
            // Recover corresponding recording pipeline
            // --------------------------------------------------------

            if (recording_manager_ == nullptr)
            {
                Logger::error(
                    "[WATCHDOG] RecordingManager unavailable."
                );

                healthy = false;
                continue;
            }

            if (!recording_manager_->recoverRecorder(camera_id))
            {
                Logger::error(
                    "[WATCHDOG] Recording recovery failed: " +
                    camera_id
                );

                healthy = false;
                continue;
            }

            recovered_recorders.insert(camera_id);

            if (event_manager_ != nullptr)
            {
                event_manager_->publish(
                    EventType::CAMERA_CONNECTED,
                    "watchdog",
                    "Camera and recording recovered: " + camera_id
                );
            }
        }
    }

    // ------------------------------------------------------------
    // Recording health and recovery
    // ------------------------------------------------------------

    if (recording_manager_ == nullptr)
    {
        Logger::error(
            "[WATCHDOG] RecordingManager unavailable."
        );

        healthy = false;
    }
    else
    {
        auto unhealthy_recorders =
            recording_manager_->getUnhealthyRecorders();

        for (const auto& recorder_id : unhealthy_recorders)
        {
            if (recovered_recorders.find(recorder_id) !=
                recovered_recorders.end())
            {
                continue;
            }

            Logger::error(
                "[WATCHDOG] Recorder unhealthy: " +
                recorder_id
            );

            if (event_manager_ != nullptr)
            {
                event_manager_->publish(
                    EventType::APPLICATION_ERROR,
                    "watchdog",
                    "Recorder is unhealthy: " + recorder_id
                );
            }

            if (!recording_manager_->recoverRecorder(recorder_id))
            {
                Logger::error(
                    "[WATCHDOG] Recorder recovery failed: " +
                    recorder_id
                );

                healthy = false;
                continue;
            }
        }
    }

    // ------------------------------------------------------------
    // Storage health
    // ------------------------------------------------------------

    if (storage_manager_ == nullptr)
    {
        Logger::error(
            "[WATCHDOG] StorageManager unavailable."
        );

        healthy = false;
    }
    else if (storage_manager_->isStorageLimitReached())
    {
        Logger::warn(
            "[WATCHDOG] Storage limit reached."
        );

        if (event_manager_ != nullptr)
        {
            event_manager_->publish(
                EventType::APPLICATION_ERROR,
                "watchdog",
                "Storage limit reached. Enforcing storage policy."
            );
        }

        if (!storage_manager_->enforceStorageLimit())
        {
            Logger::error(
                "[WATCHDOG] Failed to enforce storage limit."
            );

            healthy = false;
        }
    }

    // ------------------------------------------------------------
    // System health
    // ------------------------------------------------------------

    if (system_monitor_ == nullptr)
    {
        Logger::error(
            "[WATCHDOG] SystemMonitor unavailable."
        );

        healthy = false;
    }
    else
    {
        const double cpu_usage =
            system_monitor_->getCpuUsage();

        const double memory_usage =
            system_monitor_->getMemoryUsage();

        Logger::info(
            "[WATCHDOG] CPU: " +
            std::to_string(cpu_usage) +
            "% | Memory: " +
            std::to_string(memory_usage) +
            "%"
        );

        if (cpu_usage < 0.0)
        {
            Logger::error(
                "[WATCHDOG] Failed to read CPU usage."
            );

            healthy = false;
        }
        else if (cpu_usage >= MAX_CPU_USAGE_PERCENT)
        {
            Logger::warn(
                "[WATCHDOG] CPU usage exceeds threshold: " +
                std::to_string(cpu_usage) +
                "% >= " +
                std::to_string(MAX_CPU_USAGE_PERCENT) +
                "%"
            );

            if (event_manager_ != nullptr)
            {
                event_manager_->publish(
                    EventType::APPLICATION_ERROR,
                    "watchdog",
                    "CPU usage is above configured threshold."
                );
            }
            // High CPU usage is treated as a warning,
            // not an immediate fatal health failure.
        }

        if (memory_usage < 0.0)
        {
            Logger::error(
                "[WATCHDOG] Failed to read memory usage."
            );

            healthy = false;
        }
        else if (memory_usage >= MAX_MEMORY_USAGE_PERCENT)
        {
            Logger::warn(
                "[WATCHDOG] Memory usage exceeds threshold: " +
                std::to_string(memory_usage) +
                "% >= " +
                std::to_string(MAX_MEMORY_USAGE_PERCENT) +
                "%"
            );

            if (event_manager_ != nullptr)
            {
                event_manager_->publish(
                    EventType::APPLICATION_ERROR,
                    "watchdog",
                    "Memory usage is above configured threshold."
                );
            }

            healthy = false;

            // High memory usage is treated as a warning,
            // not an immediate fatal health failure.
        }
    }

    // ------------------------------------------------------------
    // Overall result
    // ------------------------------------------------------------

    if (healthy)
    {
        Logger::info(
            "[WATCHDOG] System healthy."
        );
    }
    else
    {
        Logger::error(
            "[WATCHDOG] System health check failed."
        );
    }

    return healthy;
}