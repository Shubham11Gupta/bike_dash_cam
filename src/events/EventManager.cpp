#include "EventManager.hpp"

#include "../logging/Logger.hpp"

void EventManager::publish(
    EventType type,
    const std::string& source,
    const std::string& message)
{
    const std::string formatted_message =
        "[EVENT] " +
        eventTypeToString(type) +
        " | Source: " +
        source +
        " | " +
        message;

    Logger::info(formatted_message);
}

std::string EventManager::eventTypeToString(
    EventType type) const
{
    switch (type)
    {
        case EventType::SYSTEM_STARTED:
            return "SYSTEM_STARTED";

        case EventType::CAMERA_CONNECTED:
            return "CAMERA_CONNECTED";

        case EventType::CAMERA_DISCONNECTED:
            return "CAMERA_DISCONNECTED";

        case EventType::CAMERA_RECOVERY_STARTED:
            return "CAMERA_RECOVERY_STARTED";

        case EventType::CAMERA_RECOVERY_SUCCESS:
            return "CAMERA_RECOVERY_SUCCESS";

        case EventType::CAMERA_RECOVERY_FAILED:
            return "CAMERA_RECOVERY_FAILED";

        case EventType::RECORDING_STARTED:
            return "RECORDING_STARTED";

        case EventType::RECORDING_STOPPED:
            return "RECORDING_STOPPED";

        case EventType::SEGMENT_CREATED:
            return "SEGMENT_CREATED";

        case EventType::STORAGE_WARNING:
            return "STORAGE_WARNING";

        case EventType::STORAGE_CLEANUP:
            return "STORAGE_CLEANUP";

        case EventType::STORAGE_FAILURE:
            return "STORAGE_FAILURE";

        case EventType::APPLICATION_ERROR:
            return "APPLICATION_ERROR";

        case EventType::SYSTEM_SHUTDOWN:
            return "SYSTEM_SHUTDOWN";

        default:
            return "UNKNOWN_EVENT";
    }
}