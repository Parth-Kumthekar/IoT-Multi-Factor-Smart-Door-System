#include "OutputHandler.hpp"
#include <iostream>

// Constructor initializes the pin numbers
OutputHandler::OutputHandler(unsigned int relayPin, unsigned int greenPin, unsigned int redPin, unsigned int buzzerPin)
    : rp(relayPin), gp(greenPin), rdp(redPin), bp(buzzerPin) {}

bool OutputHandler::init() {
    try {
        // Raspberry Pi 5 header pins are controlled by /dev/gpiochip4
        auto chip = gpiod::make_chip("/dev/gpiochip4");

        // We assign the request to the optional using std::move
        // This is the key fix for the "private constructor" error
        line_request = std::move(chip.prepare_config()
            .add_line_settings(
                {rp, gp, rdp, bp},
                gpiod::line_settings()
                    .set_direction(gpiod::line_config::direction::OUTPUT)
                    .set_output_value(gpiod::line::value::INACTIVE)
            )
            .request());

        std::cout << "GPIO Initialization Successful (v2.x - chip4)" << std::endl;
        
        // Initial State: Red LED ON to show it's locked
        // We use '->' because line_request is now an optional
        line_request->set_value(rdp, gpiod::line::value::ACTIVE);
        return true;
    } catch (const std::exception& e) {
        std::cerr << "GPIO Init Error: " << e.what() << std::endl;
        return false;
    }
}

void OutputHandler::setAccessGranted() {
    if (!line_request) return; // Safety check

    // Unlock and show Green
    line_request->set_value(rp, gpiod::line::value::ACTIVE);   // Relay ON
    line_request->set_value(gp, gpiod::line::value::ACTIVE);   // Green ON
    line_request->set_value(rdp, gpiod::line::value::INACTIVE); // Red OFF
    
    beep(150); // Single success beep
}

void OutputHandler::setAccessDenied() {
    if (!line_request) return;

    // Ensure Green is off, then flash Red/Buzzer 3 times
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