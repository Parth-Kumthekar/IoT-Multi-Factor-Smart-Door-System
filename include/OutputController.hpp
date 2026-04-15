#pragma once
#include "gpiopin.hpp"

class OutputController {
public:
    OutputController();
    void init();
    void setAccessGranted();
    void setAccessDenied();

private:
    GPIOPin ledGreen;
    GPIOPin ledRed;
    GPIOPin buzzer;
};