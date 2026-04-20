#pragma once
#include <gpiod.hpp>
#include <memory>
#include <vector>

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
     * @param state true to turn LED on, false to turn off.
     */
    void setRedLed(bool state);

    /**
     * @brief Direct control for the Green LED.
     * @param state true to turn LED on, false to turn off.
     */
    void setGreenLed(bool state);

    /**
     * @brief Direct control for the Buzzer.
     * @param state true to turn buzzer on, false to turn off.
     */
    void setBuzzer(bool state);

private:
    std::shared_ptr<gpiod::chip> chip;
    std::shared_ptr<gpiod::line_request> req;

    // GPIO numbers for Raspberry Pi 5 Header (RP1 chip)
    // These are GPIO offsets, not physical pin numbers
    unsigned int red_offset = 17;    // Physical Pin 11
    unsigned int green_offset = 27;  // Physical Pin 13
    unsigned int buzzer_offset = 22; // Physical Pin 15
};
