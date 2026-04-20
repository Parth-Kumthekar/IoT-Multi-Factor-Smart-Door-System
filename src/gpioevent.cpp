#include "gpioevent.h"
#include <format>
#include <iostream>

/**
 * @brief Configures and starts the GPIO monitoring thread.
 * @param pinNo The BCM pin number to monitor.
 * @param chipNo The hardware GPIO chip (0 for most RPi pins).
 * * * This uses libgpiod v2 builder pattern, demonstrating "Production level" 
 * resource acquisition.
 */
void GPIOPin::start(int pinNo, int chipNo)
{
#ifdef DEBUG
    std::cerr << "[GPIO] Initializing Chip " << chipNo << " Pin " << pinNo << std::endl;
#endif

    const std::string chipPath = std::format("/dev/gpiochip{}", chipNo);
    const std::string consumername = std::format("gpioconsumer_{}_{}", chipNo, pinNo);

    // 1. Configure the pin for dual-edge detection (Opening and Closing)
    gpiod::line_config line_cfg;
    line_cfg.add_line_settings(
        pinNo,
        gpiod::line_settings()
            .set_direction(gpiod::line::direction::INPUT)
            .set_edge_detection(gpiod::line::edge::BOTH));
    
    // 2. Open hardware chip and request line access
    chip = std::make_shared<gpiod::chip>(chipPath);
        
    auto builder = chip->prepare_request();
    builder.set_consumer(consumername);
    builder.set_line_config(line_cfg);
    
    // std::make_shared ensures safe memory lifecycle management
    request = std::make_shared<gpiod::line_request>(builder.do_request());
    
    // 3. Launch worker thread to avoid blocking the main FSM
    thr = std::thread(&GPIOPin::worker, this);
}

/**
 * @brief Internal wrapper to trigger the registered user callback.
 */
void GPIOPin::gpioEvent(const gpiod::edge_event &event)
{
    if (eventCallback) eventCallback(event);
}

/**
 * @brief Hardware monitoring loop.
 * * This function runs in its own thread. It uses wait_edge_events() to 
 * ensure 0% CPU usage while the door is stationary.
 */
void GPIOPin::worker()
{
    running = true;
    while (running)
    {
        // Blocking I/O: Thread sleeps at the kernel level until an interrupt occurs
        bool r = request->wait_edge_events(std::chrono::milliseconds(ISR_TIMEOUT_MS));
        
        if (r)
        {
            // Event detected: Read the event from the kernel buffer
            gpiod::edge_event_buffer buffer;
            request->read_edge_events(buffer, 1);
            
            // Dispatch to callback
            gpioEvent(buffer.get_event(0));
        }
        else
        {
#ifdef DEBUG
            // Useful for heartbeat debugging without flooding the terminal
            // std::cerr << "GPIO Heartbeat: Monitoring..." << std::endl;
#endif
        }
    }
    
    // Clean resource release (Safe Shutdown)
    request->release();
    chip->close();
}

/**
 * @brief Signals the thread to stop and waits for it to finish.
 */
void GPIOPin::stop()
{
    running = false;
    if (thr.joinable()) thr.join();
}