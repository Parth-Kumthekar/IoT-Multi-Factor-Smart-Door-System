#pragma once
#include <gpiod.hpp>

class OutputController {
public:
    bool init();

    void granted();
    void denied();

private:
    std::shared_ptr<gpiod::chip> chip;
    std::shared_ptr<gpiod::line_request> req;

    int red = 17;
    int green = 27;
    int buzzer = 22;
};