#pragma once

#include <gpiod.hpp>
#include <functional>
#include <thread>
#include <atomic>
#include <memory>

class GPIOPin {
public:
    using Callback = std::function<void(const gpiod::edge_event&)>;

    GPIOPin();
    ~GPIOPin();

    void start(int pin, int chip = 0);
    void stop();

    void registerCallback(Callback cb);

private:
    void worker();
    void gpioEvent(const gpiod::edge_event& ev);

    std::shared_ptr<gpiod::chip> chip;
    std::shared_ptr<gpiod::line_request> request;

    std::thread thr;
    std::atomic<bool> running{false};

    Callback callback;
};