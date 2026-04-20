#pragma once
#include <gpiod.hpp>
#include <functional>
#include <thread>
#include <memory>

/**
 * @class GPIOPin
 * @brief Manages a physical GPIO pin using libgpiod with asynchronous monitoring.
 * * This class abstracts the complexities of the GPIO character device. It monitors 
 * a specific pin for state changes (e.g., a reed switch opening/closing) in a 
 * dedicated background thread and notifies listeners via a callback.
 */
class GPIOPin {
public:
    /**
     * @brief Type definition for the pin-state change callback.
     * @param value The new state of the pin (usually 0 for LOW, 1 for HIGH).
     */
    using Callback = std::function<void(int value)>;

    /**
     * @brief Opens the GPIO chip and begins monitoring the specified pin.
     * @param pin The hardware pin number to monitor.
     * @param chip The index of the GPIO chip (defaults to 0).
     */
    void start(int pin, int chip = 0);

    /**
     * @brief Stops the background monitoring thread and releases hardware resources.
     */
    void stop();

    /**
     * @brief Registers a function to be executed when the pin state changes.
     * @param cb A callable object (lambda, function pointer, or std::bind).
     */
    void setCallback(Callback cb) { callback = cb; }

private:
    /**
     * @brief Internal thread function that polls for GPIO events.
     * @details This function waits for edge events and triggers the registered callback.
     */
    void worker();

    /// Smart pointer to the GPIO chip device.
    std::shared_ptr<gpiod::chip> chip;

    /// Smart pointer to the specific line request (input configuration).
    std::shared_ptr<gpiod::line_request> request;

    /// Background thread handle for the monitoring loop.
    std::thread thr;

    /// Flag used to safely exit the worker thread loop.
    bool running = false;

    /// The pin number currently assigned to this instance.
    int pinNum;

    /// The user-defined callback executed when an event is detected.
    Callback callback;
};
