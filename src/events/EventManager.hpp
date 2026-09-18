#pragma once

#include <string>

enum class EventType
{
    SYSTEM_STARTED,

    CAMERA_CONNECTED,
    CAMERA_DISCONNECTED,
    CAMERA_RECOVERY_STARTED,
    CAMERA_RECOVERY_SUCCESS,
    CAMERA_RECOVERY_FAILED,

    RECORDING_STARTED,
    RECORDING_STOPPED,
    SEGMENT_CREATED,

    STORAGE_WARNING,
    STORAGE_CLEANUP,
    STORAGE_FAILURE,

    APPLICATION_ERROR,
    SYSTEM_SHUTDOWN
};

class EventManager
{
public:
    EventManager() = default;
    ~EventManager() = default;

    void publish(
        EventType type,
        const std::string& source,
        const std::string& message);

private:
    std::string eventTypeToString(EventType type) const;
};