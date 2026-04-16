#pragma once
#include <atomic>
#include <iostream>
#include "Logger.h"

class OverrideManager {
public:
    static OverrideManager& instance() {
        static OverrideManager o;
        return o;
    }

    // Returns true if bypass-verification mode is ON
    bool isActive() const { return active_.load(); }

    void enable(const std::string& by = "Admin") {
        active_.store(true);
        Logger::instance().log(by, AccessMethod::OVERRIDE,
                               AccessResult::OVERRIDE,
                               "Override mode enabled");
        std::cout << "[Override] BYPASS ACTIVE — verification disabled\n";
    }

    void disable(const std::string& by = "Admin") {
        active_.store(false);
        Logger::instance().log(by, AccessMethod::OVERRIDE,
                               AccessResult::GRANTED,
                               "Override mode disabled");
        std::cout << "[Override] BYPASS OFF — normal verification restored\n";
    }

    void toggle(const std::string& by = "Admin") {
        active_.load() ? disable(by) : enable(by);
    }

private:
    std::atomic<bool> active_{false};
};