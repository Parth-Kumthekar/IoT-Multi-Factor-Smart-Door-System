#include "gpiopin.hpp"
#include <chrono>
#include <iostream>

/**
 * @brief Initializes the GPIO line and spawns the monitoring worker thread.
 * @param pin The BCM pin number to request.
 * @param chipNo The index of the GPIO chip (usually 0 or 4 on RPi 5).
 */
void GPIOPin::start(int pin, int chipNo)
{
    pinNum = pin;

    // Use libgpiod v2 style chip path and resource acquisition
    std::string chipPath = "/dev/gpiochip" + std::to_string(chipNo);
    chip = std::make_shared<gpiod::chip>(chipPath);

    // Configure for dual-edge detection to capture both Open and Close events
    gpiod::line_settings settings;
    settings.set_direction(gpiod::line::direction::INPUT);
    settings.set_edge_detection(gpiod::line::edge::BOTH);

    gpiod::line_config cfg;
    cfg.add_line_settings(pin, settings);

    auto builder = chip->prepare_request();
    builder.set_line_config(cfg);
    builder.set_consumer("smart_door");

    // Request the line from the kernel
    request = std::make_shared<gpiod::line_request>(builder.do_request());

    running = true;
    thr = std::thread(&GPIOPin::worker, this);
}

/**
 * @brief Background loop that monitors for hardware interrupts.
 * * Uses wait_edge_events() to suspend the thread, ensuring 0% CPU 
 * usage when the pin state is static.
 */
void GPIOPin::worker()
{
    while (running)
    {
        // 500ms timeout allows the thread to check the 'running' flag periodically
        if (request->wait_edge_events(std::chrono::milliseconds(500)))
        {
            gpiod::edge_event_buffer buf;
            request->read_edge_events(buf);

            for (unsigned int i = 0; i < buf.num_events(); i++)
            {
                auto event = buf.get_event(i);

                // Translate hardware edge type to a binary integer (1 for High/Rising, 0 for Low/Falling)
                int val = (event.type() == gpiod::edge_event::event_type::RISING_EDGE) ? 1 : 0;

                if (callback)
                    callback(val);
            }
        }
    }
}

/**
 * @brief Safely joins the worker thread before the object is destroyed.
 */
void GPIOPin::stop()
{
    running = false;

    if (thr.joinable())
        thr.join();
}