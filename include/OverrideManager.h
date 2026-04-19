#pragma once
#include <atomic>
#include <string>
#include "EventBus.h"

class OverrideManager {
public:
    static OverrideManager& instance() {
        static OverrideManager o;
        return o;
    }

    bool isActive() const { return active_.load(); }

    void enable(const std::string& by = "GUI") {
        active_.store(true);
        EventBus::instance().publish(
            AccessEvent::override_(by, "Override mode enabled"));
    }

    void disable(const std::string& by = "GUI") {
        active_.store(false);
       
        
        AccessEvent ev = AccessEvent::granted(
            AuthMethod::SYSTEM, by, 1.f, "Override mode disabled");
        EventBus::instance().publish(ev);
    }

    void toggle(const std::string& by = "GUI") {
        active_.load() ? disable(by) : enable(by);
    }

private:
    OverrideManager() = default;
    std::atomic<bool> active_{false};
};