#pragma once

#include "events/Event.hpp"

#include <cstddef>

namespace bike_dashcam::events {

class IEventManager {
public:
    virtual ~IEventManager() = default;

    virtual void publish(const Event& event) = 0;
    virtual std::size_t pendingEventCount() const = 0;
};

}  // namespace bike_dashcam::events
