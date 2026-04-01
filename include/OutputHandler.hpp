#ifndef OUTPUT_HANDLER_HPP
#define OUTPUT_HANDLER_HPP

#include <gpiod.hpp>
#include <thread>
#include <chrono>
#include <vector>

class OutputHandler {
public:
    OutputHandler(unsigned int relayPin, unsigned int greenPin, unsigned int redPin, unsigned int buzzerPin)
        : rp(relayPin), gp(greenPin), rdp(redPin), bp(buzzerPin) {}

    bool init() {
        try {
            // Pi 5 GPIOs are managed by gpiochip4
            auto chip = gpiod::make_chip("/dev/gpiochip4");

            // In v2.0, we create a "request" for all output pins at once
            line_request = chip.prepare_config()
                .add_line_settings(
                    {rp, gp, rdp, bp},
                    gpiod::line_settings()
                        .set_direction(gpiod::line_config::direction::OUTPUT)
                        .set_output_value(gpiod::line::value::INACTIVE)
                )
                .request();

            return true;
        } catch (...) { 
            return false; 
        }
    }

    void setAccessGranted() {
        // Use the request object to set multiple values
        line_request.set_value(rp, gpiod::line::value::ACTIVE);   // Unlock
        line_request.set_value(gp, gpiod::line::value::ACTIVE);   // Green ON
        line_request.set_value(rdp, gpiod::line::value::INACTIVE); // Red OFF
        beep(100);
    }

    void setAccessDenied() {
        line_request.set_value(gp, gpiod::line::value::INACTIVE);
        for(int i=0; i<3; i++) {
            line_request.set_value(rdp, gpiod::line::value::ACTIVE);
            line_request.set_value(bp, gpiod::line::value::ACTIVE);
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
            
            line_request.set_value(rdp, gpiod::line::value::INACTIVE);
            line_request.set_value(bp, gpiod::line::value::INACTIVE);
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }

    void lock() {
        line_request.set_value(rp, gpiod::line::value::INACTIVE);
        line_request.set_value(gp, gpiod::line::value::INACTIVE);
        line_request.set_value(rdp, gpiod::line::value::ACTIVE);
    }

private:
    void beep(int duration_ms) {
        line_request.set_value(bp, gpiod::line::value::ACTIVE);
        std::this_thread::sleep_for(std::chrono::milliseconds(duration_ms));
        line_request.set_value(bp, gpiod::line::value::INACTIVE);
    }

    unsigned int rp, gp, rdp, bp;
    // In v2.x, the 'line_request' holds the connection to your pins
    gpiod::line_request line_request; 
};

#endif