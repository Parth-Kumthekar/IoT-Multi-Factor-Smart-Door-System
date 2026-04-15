#include "gpiopin.hpp"
#include <iostream> 

GPIOPin::GPIOPin(int pinNum, bool output) : pinNum(pinNum), isOutput(output), running(false) {}

GPIOPin::~GPIOPin() {
    stop();
}

void GPIOPin::start(int value) {
    if (isOutput) setValue(value);
    running = true;
    if (!isOutput && callback) {
        eventThread = std::thread(&GPIOPin::eventLoop, this);
    }
}

void GPIOPin::stop() {
    running = false;
    if (eventThread.joinable()) eventThread.join();
}

void GPIOPin::setValue(int value) {
    // Replace with actual gpiod write
    std::cout << "[GPIO " << pinNum << "] Set to " << value << std::endl;
}

int GPIOPin::getValue() const {
    // Replace with actual gpiod read
    return 0;
}

void GPIOPin::registerCallback(Callback cb, int debounce_ms) {
    callback = cb;
    debounceMs = debounce_ms;
}

void GPIOPin::eventLoop() {
    int lastValue = getValue();
    while (running) {
        int val = getValue();
        if (val != lastValue) {
            std::this_thread::sleep_for(std::chrono::milliseconds(debounceMs));
            if (getValue() == val && callback) callback();
            lastValue = val;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}