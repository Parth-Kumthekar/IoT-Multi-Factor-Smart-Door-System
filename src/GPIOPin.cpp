#include "gpiopin.hpp"
#include <chrono>
#include <iostream>

/**
 * @brief Initializes the GPIO line and spawns a background monitoring thread.
 * @details Configures the pin as an input with edge detection set to BOTH (rising and falling).
 * It uses the libgpiod request builder to assign a consumer name ("smart_door") and 
 * prepares the hardware line for event polling. 
 * Updated: Added try-catch blocks and null-pointer validation to prevent crashes 
 * during hardware initialization failure.
 * * @param pin The GPIO line offset to monitor (e.g., 23).
 * @param chipNo The index of the gpiochip (e.g., 4 for Raspberry Pi 5 RP1).
 */
void GPIOPin::start(int pin, int chipNo)
{
    if (running) return; // Prevent multiple starts
    pinNum = pin;

    try {
        std::string chipPath = "/dev/gpiochip" + std::to_string(chipNo);
        chip = std::make_shared<gpiod::chip>(chipPath);

        gpiod::line_settings settings;
        settings.set_direction(gpiod::line::direction::INPUT);
        settings.set_edge_detection(gpiod::line::edge::BOTH);

        gpiod::line_config cfg;
        cfg.add_line_settings(pin, settings);

        auto builder = chip->prepare_request();
        builder.set_line_config(cfg);
        builder.set_consumer("smart_door_sensor");

        // Request the line from the hardware
        request = std::make_shared<gpiod::line_request>(builder.do_request());

        // CRITICAL SAFETY CHECK: Only start thread if hardware request succeeded
        if (request) {
            running = true;
            thr = std::thread(&GPIOPin::worker, this);
        }
    } catch (const std::exception& e) {
        // Log the error instead of crashing the whole system
        std::cerr << "GPIOPin Error: Failed to initialize pin " << pin 
                  << " on chip " << chipNo << ": " << e.what() << std::endl;
        request.reset(); 
    }
}

/**
 * @brief Internal worker loop that polls for hardware interrupts.
 * @details This function runs in a dedicated thread, blocking for up to 500ms on 
 * wait_edge_events(). When an interrupt occurs, it reads the event buffer, 
 * determines if the edge was RISING (1) or FALLING (0), and executes the 
 * registered callback.
 */
void GPIOPin::worker()
{
    // Second safety check: ensure the request pointer is valid before entering loop
    if (!request) return;

    while (running)
    {
        try {
            // Blocking wait to minimize CPU usage (500ms timeout)
            if (request->wait_edge_events(std::chrono::milliseconds(500)))
            {
                gpiod::edge_event_buffer buf;
                // Read up to 16 events from the buffer
                request->read_edge_events(buf);

                for (unsigned int i = 0; i < buf.num_events(); i++)
                {
                    auto event = buf.get_event(i);

                    // Map RISING_EDGE to 1 and FALLING_EDGE to 0
                    int val = (event.type() == gpiod::edge_event::event_type::RISING_EDGE) ? 1 : 0;

                    if (callback) {
                        callback(val);
                    }
                }
            }
        } catch (const std::exception& e) {
            std::cerr << "GPIOPin Worker Error: " << e.what() << std::endl;
            running = false; // Exit loop on hardware fault
        }
    }
}

/**
 * @brief Safely terminates the monitoring thread and releases hardware.
 * @details Sets the running flag to false and joins the worker thread to ensure
 * all resources are cleaned up before the object is destroyed.
 */
void GPIOPin::stop()
{
    running = false;

    if (thr.joinable()) {
        thr.join();
    }

    // Explicitly release the GPIO request and chip
    request.reset();
    chip.reset();
}
