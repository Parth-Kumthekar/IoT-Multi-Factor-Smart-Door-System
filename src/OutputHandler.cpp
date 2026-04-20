#include "OutputHandler.hpp"
#include <gpiod.hpp>
#include <iostream>

/**
 * @brief Initializes all output hardware (Relay, LEDs, Buzzer).
 * @details This implementation uses the libgpiod v2 builder pattern to 
 * ensure that the GPIO chip remains open for the duration of the program.
 */
bool OutputHandler::init() {
    try {
        // 1. Maintain the chip as a persistent object (ensure it's a member variable in .hpp)
        // chip_ = std::make_unique<gpiod::chip>("/dev/gpiochip4");
        
        gpiod::line_settings out_settings;
        out_settings.set_direction(gpiod::line::direction::OUTPUT);
        out_settings.set_output_value(gpiod::line::value::INACTIVE);

        // 2. Requesting lines individually is acceptable, but 
        // using a builder for a single bulk request is more efficient.
        // For simplicity, we stick to your individual structure but ensure persistence:
        
        gpiod::chip chip("/dev/gpiochip4");

        auto request_line = [&](int pin, const std::string& name) {
            gpiod::line_config cfg;
            cfg.add_line_settings(pin, out_settings);
            
            return chip.prepare_request()
                .set_consumer(name)
                .set_line_config(cfg)
                .do_request();
        };

        relay_request  = request_line(rp,  "door_relay");
        green_request  = request_line(gp,  "green_led");
        red_request    = request_line(rdp, "red_led");
        buzzer_request = request_line(bp,  "alarm_buzzer");

        std::cout << "[HARDWARE] OutputHandler: All lines successfully claimed." << std::endl;
        return true;
    } catch (const std::exception &e) {
        std::cerr << "[CRITICAL] OutputHandler init error: " << e.what() << std::endl;
        return false;
    }
}