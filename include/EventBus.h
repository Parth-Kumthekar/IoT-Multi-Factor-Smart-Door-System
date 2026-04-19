#pragma once
#include <functional>
#include <vector>
#include <mutex>
#include <string>
#include "AccessEvent.h"


using EventCallback = std::function<void(const AccessEvent&)>;

class EventBus {
public:
    static EventBus& instance() {
        static EventBus bus;
        return bus;
    }

    // Register
    std::size_t subscribe(EventCallback cb) {
        std::lock_guard<std::mutex> lk(mutex_);
        callbacks_.push_back(std::move(cb));
        return callbacks_.size() - 1;
    }

    
    void publish(AccessEvent ev) {
        std::lock_guard<std::mutex> lk(mutex_);
        for (auto& cb : callbacks_)
            cb(ev);
    }

private:
    EventBus() = default;
    std::mutex                  mutex_;
    std::vector<EventCallback>  callbacks_;
};