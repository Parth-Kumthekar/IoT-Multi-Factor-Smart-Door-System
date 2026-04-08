#pragma once
#include <gpiod.hpp>
#include <memory>

class OutputController {
private:
    std::shared_ptr<gpiod::chip> chip;

    std::shared_ptr<gpiod::line_request> green;
    std::shared_ptr<gpiod::line_request> red;
    std::shared_ptr<gpiod::line_request> buzzer;

    std::shared_ptr<gpiod::line_request> createOutput(int pin);
    void set(std::shared_ptr<gpiod::line_request> line, bool val);

public:
    void init();

    void accessGranted();
    void accessDenied();
};