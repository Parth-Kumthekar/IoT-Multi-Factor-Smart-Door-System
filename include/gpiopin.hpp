#pragma once

#include <gpiod.hpp>
#include <memory>
#include <thread>
#include <functional>
#include <atomic>

#define ISR_TIMEOUT_MS 1000

class GPIOPin {
private:
    std::shared_ptr<gpiod::chip> chip;
    std::shared_ptr<gpiod::line_request> request;

    std::thread thr;
    std::atomic<bool> running{false};

    std::function<void(const gpiod::edge_event&)> eventCallback;

    void worker();
    void gpioEvent(const gpiod::edge_event& event);

public:
    void start(int pinNo, int chipNo = 0);
    void stop();

    void registerCallback(std::function<void(const gpiod::edge_event&)> cb) {
        eventCallback = cb;
    }
};