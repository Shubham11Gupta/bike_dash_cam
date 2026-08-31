#include "events/EventManager.hpp"

namespace bike_dashcam::events {

void EventManager::publish(const Event& event) {
    std::lock_guard<std::mutex> lock(mutex_);
    pending_events_.push_back(event);
}

std::size_t EventManager::pendingEventCount() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return pending_events_.size();
}

std::vector<Event> EventManager::pendingEvents() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return pending_events_;
}

}  // namespace bike_dashcam::events
