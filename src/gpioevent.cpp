#include "gpioevent.h"
#include <format>

/**
 * @brief Initializes and starts the GPIO monitoring thread.
 * @details Configures the specified pin on the given chip for bidirectional edge detection 
 * (both RISING and FALLING). It uses libgpiod's request builder to claim the hardware 
 * line before spawning the worker thread.
 * * @param pinNo The GPIO line offset on the chip.
 * @param chipNo The integer ID of the gpiochip (e.g., 0 for /dev/gpiochip0).
 */
void GPIOPin::start(int pinNo,
        int chipNo)
{

#ifdef DEBUG
    std::cerr << "Init" << std::endl;
#endif

    const std::string chipPath = std::format("/dev/gpiochip{}", chipNo);
    const std::string consumername = std::format("gpioconsumer_{}_{}", chipNo, pinNo);

    // Config the pin as input and detecting falling and rising edges
    gpiod::line_config line_cfg;
    line_cfg.add_line_settings(
               pinNo,
               gpiod::line_settings()
               .set_direction(gpiod::line::direction::INPUT)
               .set_edge_detection(gpiod::line::edge::BOTH));
    
    chip = std::make_shared<gpiod::chip>(chipPath);
        
    auto builder = chip->prepare_request();
    builder.set_consumer(consumername);
    builder.set_line_config(line_cfg);
    request = std::make_shared<gpiod::line_request>(builder.do_request());
    
    thr = std::thread(&GPIOPin::worker, this);
}

/**
 * @brief Internal wrapper to trigger the user-defined callback.
 * @param event The edge_event object containing the line offset and edge type.
 */
void GPIOPin::gpioEvent(const gpiod::edge_event &event)
{
    if (eventCallback) eventCallback(event);
}

/**
 * @brief Background worker loop for edge event monitoring.
 * @details This function runs in a dedicated thread. It utilizes blocking I/O 
 * (wait_edge_events) to minimize CPU usage. When an edge is detected, it reads 
 * the event into a buffer and dispatches it via gpioEvent().
 * * @note The loop duration is governed by ISR_TIMEOUT_MS before checking the 'running' flag.
 */
void GPIOPin::worker()
{
    running = true;
    while (running)
    {
        // blocking I/O: thread goes to sleep till an event has happened.
        bool r = request->wait_edge_events(std::chrono::milliseconds(ISR_TIMEOUT_MS));
        if (r)
        {
            gpiod::edge_event_buffer buffer;
            request->read_edge_events(buffer, 1);
            // callback
            gpioEvent(buffer.get_event(0));
        }
        else
        {
#ifdef DEBUG
            std::cerr << "Timeout" << std::endl;
#endif
        }
    }
    // Resource cleanup upon loop exit
    request->release();
    chip->close();
}

/**
 * @brief Signals the worker thread to stop and joins it.
 * @details Sets the atomic 'running' flag to false and blocks until the thread terminates.
 */
void GPIOPin::stop()
{
    running = false;
    if (thr.joinable()) thr.join();
}
