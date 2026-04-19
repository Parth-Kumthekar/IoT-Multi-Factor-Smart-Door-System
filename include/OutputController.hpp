#pragma once
#include <gpiod.hpp>
#include <memory>

class OutputController {
public:
    /**
     * @brief Initializes gpiochip4 and requests lines for Red, Green, and Buzzer.
     * @return true if hardware was successfully claimed, false otherwise.
     */
    bool init();

    /**
     * @brief High-level command for valid access: Green LED ON, others OFF.
     */
    void granted();

    /**
     * @brief High-level command for invalid access: Red LED + Buzzer ON.
     */
    void denied();

    /**
     * @brief Direct control for the Red LED. 
     * Used for the Reed Switch hardware mirror debug.
     * @param state true to turn LED on, false to turn off.
     */
    void setRedLed(bool state); 

private:
    std::shared_ptr<gpiod::chip> chip;
    std::shared_ptr<gpiod::line_request> req;

    // GPIO numbers for Raspberry Pi 5 Header (RP1)
    int red = 17;    // Physical Pin 11
    int green = 27;  // Physical Pin 13
    int buzzer = 22; // Physical Pin 15
};