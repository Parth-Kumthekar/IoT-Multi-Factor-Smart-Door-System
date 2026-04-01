#include "OutputHandler.hpp"
#include <iostream>

OutputHandler::OutputHandler(unsigned int relayPin, unsigned int greenPin, unsigned int redPin, unsigned int buzzerPin)
    : rp(relayPin), gp(greenPin), rdp(redPin), bp(buzzerPin) {}

bool OutputHandler::init() {
    try {
        gpiod::chip chip("/dev/gpiochip4");

        line_request = std::move(chip.prepare_config()
            .add_line_settings(
                {rp, gp, rdp, bp},
                gpiod::line_settings()
                    .set_direction(gpiod::line::direction::OUTPUT)
                    .set_output_value(gpiod::line::value::INACTIVE)
            )
            .request());

        std::cout << "GPIO Outputs Initialized." << std::endl;
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Output Hardware Init Error: " << e.what() << std::endl;
        return false;
    }
}

void OutputHandler::setAccessGranted() {
    if (!line_request) return;
    line_request->set_value(rp, gpiod::line::value::ACTIVE);   
    line_request->set_value(gp, gpiod::line::value::ACTIVE);   
    line_request->set_value(rdp, gpiod::line::value::INACTIVE); 
    beep(150);
}

void OutputHandler::setAccessDenied() {
    if (!line_request) return;
    line_request->set_value(gp, gpiod::line::value::INACTIVE);
    for(int i = 0; i < 3; i++) {
        line_request->set_value(rdp, gpiod::line::value::ACTIVE);
        line_request->set_value(bp, gpiod::line::value::ACTIVE);
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        line_request->set_value(rdp, gpiod::line::value::INACTIVE);
        line_request->set_value(bp, gpiod::line::value::INACTIVE);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

void OutputHandler::lock() {
    if (!line_request) return;
    line_request->set_value(rp, gpiod::line::value::INACTIVE);
    line_request->set_value(gp, gpiod::line::value::INACTIVE);
    line_request->set_value(rdp, gpiod::line::value::ACTIVE);
}

void OutputHandler::beep(int duration_ms) {
    if (!line_request) return;
    line_request->set_value(bp, gpiod::line::value::ACTIVE);
    std::this_thread::sleep_for(std::chrono::milliseconds(duration_ms));
    line_request->set_value(bp, gpiod::line::value::INACTIVE);
}