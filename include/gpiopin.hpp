#pragma once
#include <functional>
#include <thread>
#include <atomic>
#include <chrono>
#include <gpiod.hpp>

class GPIOPin {
public:
    using Callback = std::function<void()>;

    GPIOPin(int pinNum, bool output = false);
    ~GPIOPin();

    void start(int value = 0);
    void stop();
    void setValue(int value);
    int getValue() const;
    void registerCallback(Callback cb, int debounce_ms = 50);

private:
    int pinNum; 
    bool isOutput;
    std::atomic<bool> running;
    std::thread eventThread;
    Callback callback;
    int debounceMs;

    void eventLoop();
};