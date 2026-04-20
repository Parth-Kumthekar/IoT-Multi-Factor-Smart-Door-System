#pragma once
#include <gpiod.hpp>
#include <functional>
#include <thread>
#include <memory>

/**
 * @class GPIOPin
 * @brief Modern Linux GPIO abstraction using libgpiod.
 * * This class manages hardware pin interaction on the Raspberry Pi. It 
 * encapsulates a background worker thread that monitors for edge transitions, 
 * allowing the system to respond to physical inputs via asynchronous callbacks 
 * rather than CPU-intensive polling.
 */
class GPIOPin {
public:
    /** @brief Function signature for GPIO event notifications. */
    using Callback = std::function<void(int value)>;

    /**
     * @brief Initializes the GPIO pin and starts the edge-detection worker thread.
     * @param pin The BCM pin number on the Raspberry Pi header.
     * @param chip The GPIO chip index (defaulting to 0 for RPi).
     */
    void start(int pin, int chip = 0);

    /**
     * @brief Safely releases the GPIO line and joins the worker thread.
     */
    void stop();

    /**
     * @brief Registers a handler to be executed when a pin state change occurs.
     * @param cb The callback function (e.g., a lambda or class member).
     */
    void setCallback(Callback cb) { callback = cb; }

private:
    /**
     * @brief Background loop that waits for hardware edge events.
     * * Utilizing libgpiod's event waiting mechanism, this thread remains 
     * suspended until a hardware transition occurs, ensuring high efficiency.
     */
    void worker();

    /** @brief Shared reference to the GPIO chip resource. */
    std::shared_ptr<gpiod::chip> chip;

    /** @brief Shared reference to the active line request/configuration. */
    std::shared_ptr<gpiod::line_request> request;

    /** @brief The dedicated thread for hardware event monitoring. */
    std::thread thr;

    /** @brief Atomic-like flag for controlling the worker thread lifecycle. */
    bool running = false;

    /** @brief The physical pin number currently being managed. */
    int pinNum;

    /** @brief The client-provided handler for pin events. */
    Callback callback;
};