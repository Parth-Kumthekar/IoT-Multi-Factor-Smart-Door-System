#include "OutputHandler.hpp"
#include <iostream>

// The constructor initializes the pin numbers
OutputHandler::OutputHandler(unsigned int relayPin, unsigned int greenPin, unsigned int redPin, unsigned int buzzerPin)
    : rp(relayPin), gp(greenPin), rdp(redPin), bp(buzzerPin) {}

bool OutputHandler::init() {
    try {
        // Raspberry Pi 5 header pins are controlled by gpiochip4
        auto chip = gpiod::make_chip("/dev/gpiochip4");

        // We "request" all 4 pins at once as outputs. 
        // We set them to INACTIVE (0) by default so the door starts LOCKED.
        line_request = chip.prepare_config()
            .add_line_settings(
                {rp, gp, rdp, bp},
                gpiod::line_settings()
                    .set_direction(gpiod::line_config::direction::OUTPUT)
                    .set_output_value(gpiod::line::value::INACTIVE)
            )
            .request();

        std::cout << "GPIO Initialization Successful (v2.x - chip4)" << std::endl;
        
        // Initial State: Red LED ON to show it's locked
        line_request.set_value(rdp, gpiod::line::value::ACTIVE);
        return true;
    } catch (const std::exception& e) {
        std::cerr << "GPIO Init Error: " << e.what() << std::endl;
        return false;
    }
}

void OutputHandler::setAccessGranted() {
    // Unlock and show Green
    line_request.set_value(rp, gpiod::line::value::ACTIVE);   // Relay ON
    line_request.set_value(gp, gpiod::line::value::ACTIVE);   // Green ON
    line_request.set_value(rdp, gpiod::line::value::INACTIVE); // Red OFF
    
    beep(150); // Single success beep
}

void OutputHandler::setAccessDenied() {
    // Ensure Green is off, then flash Red/Buzzer 3 times
    line_request.set_value(gp, gpiod::line::value::INACTIVE);
    
    for(int i = 0; i < 3; i++) {
        line_request.set_value(rdp, gpiod::line::value::ACTIVE);
        line_request.set_value(bp, gpiod::line::value::ACTIVE);
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        
        line_request.set_value(rdp, gpiod::line::value::INACTIVE);
        line_request.set_value(bp, gpiod::line::value::INACTIVE);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

void OutputHandler::lock() {
    // Return to secure state
    line_request.set_value(rp, gpiod::line::value::INACTIVE);   // Relay OFF (Locked)
    line_request.set_value(gp, gpiod::line::value::INACTIVE);   // Green OFF
    line_request.set_value(rdp, gpiod::line::value::ACTIVE);    // Red ON
}

void OutputHandler::beep(int duration_ms) {
    line_request.set_value(bp, gpiod::line::value::ACTIVE);
    std::this_thread::sleep_for(std::chrono::milliseconds(duration_ms));
    line_request.set_value(bp, gpiod::line::value::INACTIVE);
}