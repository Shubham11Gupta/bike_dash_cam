#include "Watchdog.hpp"

#include <iostream>
#include <unordered_set>

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

    // ------------------------------------------------------------
    // Camera health and recovery
    // ------------------------------------------------------------

    if (camera_manager_ == nullptr)
    {
        std::cerr
            << "[WATCHDOG] CameraManager unavailable."
            << std::endl;

        healthy = false;
    }
    else
    {
        auto unhealthy_cameras =
            camera_manager_->getUnhealthyCameras();

        for (const auto& camera_id : unhealthy_cameras)
        {
            std::cerr
                << "[WATCHDOG] Camera unhealthy: "
                << camera_id
                << std::endl;

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
                std::cerr
                    << "[WATCHDOG] Camera recovery failed: "
                    << camera_id
                    << std::endl;

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
                std::cerr
                    << "[WATCHDOG] RecordingManager unavailable."
                    << std::endl;

                healthy = false;
                continue;
            }

            if (!recording_manager_->recoverRecorder(camera_id))
            {
                std::cerr
                    << "[WATCHDOG] Recording recovery failed: "
                    << camera_id
                    << std::endl;

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
        std::cerr
            << "[WATCHDOG] RecordingManager unavailable."
            << std::endl;

        healthy = false;
    }
    else
    {
        auto unhealthy_recorders =
            recording_manager_->getUnhealthyRecorders();

        for (const auto& recorder_id : unhealthy_recorders)
        {
            if (recovered_recorders.find(recorder_id) != recovered_recorders.end())
            {
                continue;
            }

            std::cerr
                << "[WATCHDOG] Recorder unhealthy: "
                << recorder_id
                << std::endl;

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
                std::cerr
                    << "[WATCHDOG] Recorder recovery failed: "
                    << recorder_id
                    << std::endl;

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
        std::cerr
            << "[WATCHDOG] StorageManager unavailable."
            << std::endl;

        healthy = false;
    }
    else if (storage_manager_->isStorageLimitReached())
    {
        std::cerr
            << "[WATCHDOG] Storage limit reached."
            << std::endl;

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
            std::cerr
                << "[WATCHDOG] Failed to enforce storage limit."
                << std::endl;

            healthy = false;
        }
    }

    // ------------------------------------------------------------
    // System health
    // ------------------------------------------------------------

    if (system_monitor_ == nullptr)
    {
        std::cerr
            << "[WATCHDOG] SystemMonitor unavailable."
            << std::endl;

        healthy = false;
    }
    else
    {
        const double cpu_usage =
            system_monitor_->getCpuUsage();

        const double memory_usage =
            system_monitor_->getMemoryUsage();

        std::cout
            << "[WATCHDOG] CPU: "
            << cpu_usage
            << "% | Memory: "
            << memory_usage
            << "%"
            << std::endl;

        if (cpu_usage < 0.0)
        {
            std::cerr
                << "[WATCHDOG] Failed to read CPU usage."
                << std::endl;

            healthy = false;
        }
        else if (cpu_usage >= MAX_CPU_USAGE_PERCENT)
        {
            std::cerr
                << "[WATCHDOG] CPU usage exceeds threshold: "
                << cpu_usage
                << "% >= "
                << MAX_CPU_USAGE_PERCENT
                << "%"
                << std::endl;

            healthy = false;
        }

        if (memory_usage < 0.0)
        {
            std::cerr
                << "[WATCHDOG] Failed to read memory usage."
                << std::endl;

            healthy = false;
        }
        else if (memory_usage >= MAX_MEMORY_USAGE_PERCENT)
        {
            std::cerr
                << "[WATCHDOG] Memory usage exceeds threshold: "
                << memory_usage
                << "% >= "
                << MAX_MEMORY_USAGE_PERCENT
                << "%"
                << std::endl;

            healthy = false;
        }
    }

    // ------------------------------------------------------------
    // Overall result
    // ------------------------------------------------------------

    if (healthy)
    {
        std::cout
            << "[WATCHDOG] System healthy."
            << std::endl;
    }
    else
    {
        std::cerr
            << "[WATCHDOG] System health check failed."
            << std::endl;
    }

    return healthy;
}