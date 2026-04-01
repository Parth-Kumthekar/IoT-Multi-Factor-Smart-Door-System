#ifndef GPIOHANDLER_HPP
#define GPIOHANDLER_HPP

#include <gpiod.hpp>
#include <string>
#include <iostream>

class GPIOHandler {
public:
    // Usually "gpiochip4" is the RP1 controller on Pi 5
    GPIOHandler(unsigned int pin, const std::string& chipName = "gpiochip4") 
        : pinNum(pin), chipName(chipName) {}

    // Configure as Output
    bool setupOutput() {
        try {
            chip = gpiod::make_chip(chipName);
            line = chip.get_line(pinNum);
            line.request({"Pi5_Output", gpiod::line_request::DIRECTION_OUTPUT, 0}, 0);
            return true;
        } catch (const std::exception& e) {
            std::cerr << "Export Error: " << e.what() << std::endl;
            return false;
        }
    }

    // Configure as Input
    bool setupInput() {
        try {
            chip = gpiod::make_chip(chipName);
            line = chip.get_line(pinNum);
            line.request({"Pi5_Input", gpiod::line_request::DIRECTION_INPUT, 0});
            return true;
        } catch (const std::exception& e) {
            std::cerr << "Export Error: " << e.what() << std::endl;
            return false;
        }
    }

    void write(int value) {
        line.set_value(value);
    }

    int read() {
        return line.get_value();
    }

private:
    unsigned int pinNum;
    std::string chipName;
    gpiod::chip chip;
    gpiod::line line;
};

#endif