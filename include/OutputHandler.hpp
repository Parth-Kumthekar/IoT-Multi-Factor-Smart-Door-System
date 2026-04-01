#ifndef OUTPUT_HANDLER_HPP
#define OUTPUT_HANDLER_HPP

#include <gpiod.hpp>
#include <thread>
#include <chrono>

class OutputHandler {
public:
    OutputHandler(unsigned int relayPin, unsigned int greenPin, unsigned int redPin, unsigned int buzzerPin)
        : rp(relayPin), gp(greenPin), rdp(redPin), bp(buzzerPin) {}

    bool init() {
        try {
            chip = gpiod::make_chip("gpiochip4");
            // Request all pins as outputs
            relay = chip.get_line(rp);
            green = chip.get_line(gp);
            red = chip.get_line(rdp);
            buzzer = chip.get_line(bp);

            relay.request({"Relay", gpiod::line_request::DIRECTION_OUTPUT, 0}, 0);
            green.request({"GreenLED", gpiod::line_request::DIRECTION_OUTPUT, 0}, 0);
            red.request({"RedLED", gpiod::line_request::DIRECTION_OUTPUT, 0}, 0);
            buzzer.request({"Buzzer", gpiod::line_request::DIRECTION_OUTPUT, 0}, 0);
            
            return true;
        } catch (...) { return false; }
    }

    void setAccessGranted() {
        relay.set_value(1);  // Unlock Solenoid
        green.set_value(1);  // Green LED On
        red.set_value(0);
        beep(100);           // Short success beep
    }

    void setAccessDenied() {
        green.set_value(0);
        for(int i=0; i<3; i++) { // Flash Red & Beep 3 times
            red.set_value(1);
            buzzer.set_value(1);
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
            red.set_value(0);
            buzzer.set_value(0);
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }

    void lock() {
        relay.set_value(0);
        green.set_value(0);
        red.set_value(1); // Red means locked
    }

private:
    void beep(int duration_ms) {
        buzzer.set_value(1);
        std::this_thread::sleep_for(std::chrono::milliseconds(duration_ms));
        buzzer.set_value(0);
    }

    unsigned int rp, gp, rdp, bp;
    gpiod::chip chip;
    gpiod::line relay, green, red, buzzer;
};

#endif