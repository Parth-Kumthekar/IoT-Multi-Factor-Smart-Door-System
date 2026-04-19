#pragma once

#include <gpiod.hpp>
#include <memory>

class OutputController {
public:
    bool init();

    void accessGranted();
    void accessDenied();

private:
    std::shared_ptr<gpiod::chip> chip;
    std::shared_ptr<gpiod::line_request> req;

    int green = 17;
    int red   = 27;
    int buzzer = 22;
};