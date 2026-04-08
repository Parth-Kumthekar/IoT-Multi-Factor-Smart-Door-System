#include "DoorController.hpp"
#include <gpiod.hpp>
#include <iostream>

bool DoorController::initialize() {
    try {
        gpiod::chip chip("/dev/gpiochip4");

        // Configure input line settings
        gpiod::line_settings in_settings;
        in_settings.set_direction(gpiod::line::direction::INPUT);

        // Request each input line individually
        reed_request = gpiod::line_request(chip.get_line(reedPinNum), in_settings);

        std::cout << "DoorController GPIO initialized successfully." << std::endl;
        return true;
    } catch (const std::exception &e) {
        std::cerr << "DoorController init error: " << e.what() << std::endl;
        return false;
    }
}