#include "OutputHandler.hpp"
#include <iostream>

// Constructor initializes the pin numbers
OutputHandler::OutputHandler(unsigned int relayPin, unsigned int greenPin, unsigned int redPin, unsigned int buzzerPin)
    : rp(relayPin), gp(greenPin), rdp(redPin), bp(buzzerPin) {}

bool OutputHandler::init() {
    try {
        // 1. Open the GPIO chip (Pi 5 uses chip 4 for the 40-pin header)
        gpiod::chip chip("/dev/gpiochip4");

        // 2. Define the settings for our output pins
        auto settings = gpiod::line_settings()
            .set_direction(gpiod::line::direction::OUTPUT)
            .set_output_value(gpiod::line::value::INACTIVE);

        // 3. Create a line configuration and map our pins to those settings
        auto line_cfg = gpiod::line_config();
        line_cfg.add_line_settings({rp, gp, rdp, bp}, settings);

        // 4. Request the lines and MOVE the request into our class member (std::optional)
        // This replaces the old .prepare_config().request() syntax
        line_request = std::move(chip.request_lines(line_cfg));

        std::cout << "GPIO Outputs Initialized successfully (v2.2.1)" << std::endl;
        
        // Start with the Red LED active to indicate the door is locked
        line_request->set_value(rdp, gpiod::line::value::ACTIVE);
        
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Output Hardware Init Error: " << e.what() << std::endl;
        return false;
    }
}

void OutputHandler::setAccessGranted() {
    if (!line_request) return;

    // Unlock door and change LEDs
    line_request->set_value(rp, gpiod::line::value::ACTIVE);    // Relay ON
    line_request->set_value(gp, gpiod::line::value::ACTIVE);    // Green ON
    line_request->set_value(rdp, gpiod::line::value::INACTIVE); // Red OFF
    
    beep(150); // Audible confirmation
}

void OutputHandler::setAccessDenied() {
    if (!line_request) return;

    // Ensure Green is off, then flash Red and Buzzer 3 times
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

    // Return to secure state
    line_request->set_value(rp, gpiod::line::value::INACTIVE);   // Relay OFF (Locked)
    line_request->set_value(gp, gpiod::line::value::INACTIVE);   // Green OFF
    line_request->set_value(rdp, gpiod::line::value::ACTIVE);    // Red ON
}

void OutputHandler::beep(int duration_ms) {
    if (!line_request) return;

    line_request->set_value(bp, gpiod::line::value::ACTIVE);
    std::this_thread::sleep_for(std::chrono::milliseconds(duration_ms));
    line_request->set_value(bp, gpiod::line::value::INACTIVE);
}