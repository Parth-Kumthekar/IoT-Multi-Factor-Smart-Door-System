#pragma once
#include <gpiod.hpp>
#include <functional>
#include <thread>
#include <memory>

class GPIOPin {
public:
    using Callback = std::function<void(int value)>;

    void start(int pin, int chip = 0);
    void stop();

    void setCallback(Callback cb) { callback = cb; }

private:
    void worker();

    std::shared_ptr<gpiod::chip> chip;
    std::shared_ptr<gpiod::line_request> request;
    std::thread thr;
    bool running = false;

    int pinNum;
    Callback callback;
};