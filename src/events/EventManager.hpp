#pragma once

#include "events/IEventManager.hpp"

#include <mutex>
#include <vector>

namespace bike_dashcam::events {

class EventManager : public IEventManager {
public:
    void publish(const Event& event) override;
    std::size_t pendingEventCount() const override;
    std::vector<Event> pendingEvents() const;

private:
    mutable std::mutex mutex_;
    std::vector<Event> pending_events_;
};

}  // namespace bike_dashcam::events
