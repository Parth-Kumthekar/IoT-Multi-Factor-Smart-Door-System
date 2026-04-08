#include "DoorController.hpp"
#include <iostream>

DoorController::DoorController(unsigned int reedPin)
    : reedPinNum(reedPin) {}

bool DoorController::initialize() {
    try {
        gpiod::chip chip("/dev/gpiochip4");

        gpiod::line_settings settings;
        settings.set_direction(gpiod::line::direction::INPUT);
        settings.set_bias(gpiod::line::bias::PULL_UP);

        gpiod::line_config line_cfg;
        gpiod::line::offsets offsets{reedPinNum};
        line_cfg.add_line_settings(offsets, settings);

        auto lines = chip.get_lines(offsets);

        reed_request.emplace();
        reed_request->request(lines, gpiod::line_request::FLAG_BIAS_PULL_UP);

        std::cout << "Door Controller GPIO initialized" << std::endl;
        return true;

    } catch (const std::exception &e) {
        std::cerr << "DoorController Init Error: " << e.what() << std::endl;
        return false;
    }
}