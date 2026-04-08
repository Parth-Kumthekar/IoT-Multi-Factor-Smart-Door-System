#include "OutputHandler.hpp"
#include <iostream>
#include <thread>
#include <chrono>

OutputHandler::OutputHandler(unsigned int relayPin, unsigned int greenPin,
                             unsigned int redPin, unsigned int buzzerPin)
    : rp(relayPin), gp(greenPin), rdp(redPin), bp(buzzerPin) {}

bool OutputHandler::init() {
    try {
        gpiod::chip chip("/dev/gpiochip4");

        // Configure pins as OUTPUT
        gpiod::line_settings settings;
        settings.set_direction(gpiod::line::direction::OUTPUT);
        settings.set_output_value(gpiod::line::value::INACTIVE);

        gpiod::line_config line_cfg;
        gpiod::line::offsets offsets{rp, gp, rdp, bp};
        line_cfg.add_line_settings(offsets, settings);

        // Get lines from chip
        auto lines = chip.get_lines(offsets);

        // Request the lines
        line_request.emplace();  // default constructor
        line_request->request(lines, gpiod::line_request::FLAG_OPEN_DRAIN);

        // Start with Red LED active (door locked)
        line_request->set_value(rdp, gpiod::line::value::ACTIVE);

        std::cout << "GPIO Outputs Initialized successfully" << std::endl;
        return true;

    } catch (const std::exception &e) {
        std::cerr << "Output Hardware Init Error: " << e.what() << std::endl;
        return false;
    }
}