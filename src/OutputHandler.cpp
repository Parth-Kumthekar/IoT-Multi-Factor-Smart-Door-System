#include "OutputHandler.hpp"
#include <gpiod.hpp>
#include <iostream>

/**
 * @brief Initializes the GPIO hardware for all output peripherals.
 * @details Specifically targets `/dev/gpiochip4` to claim lines for the 
 * door relay, buzzer, and status LEDs. Each line is requested individually 
 * with a default output value of 0 (LOW) to ensure the system starts 
 * in a secure, silent state.
 * * @return true if all GPIO lines were successfully requested.
 * @return false if the chip could not be opened or a line is already in use.
 */
bool OutputHandler::init() {
    try {
        // Open GPIO chip
        gpiod::chip chip("/dev/gpiochip4");

        // Configure line settings for outputs
        gpiod::line_settings out_settings;
        out_settings.set_direction(gpiod::line::direction::OUTPUT);
        out_settings.set_output_value(0);  // Start LOW

        // Request each output line individually
        relay_request = gpiod::line_request(chip.get_line(rp), out_settings);
        green_request = gpiod::line_request(chip.get_line(gp), out_settings);
        red_request   = gpiod::line_request(chip.get_line(rdp), out_settings);
        buzzer_request= gpiod::line_request(chip.get_line(bp), out_settings);

        std::cout << "OutputHandler GPIO initialized successfully." << std::endl;
        return true;
    } catch (const std::exception &e) {
        std::cerr << "OutputHandler init error: " << e.what() << std::endl;
        return false;
    }
}
