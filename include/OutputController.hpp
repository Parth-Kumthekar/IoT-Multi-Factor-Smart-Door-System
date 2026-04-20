#pragma once
#include <gpiod.hpp>
#include <string>

/**
 * @class OutputController
 * @brief Manages physical feedback components (LEDs and Buzzer).
 */
class OutputController {
public:
    OutputController() = default;

    /**
     * @brief Initializes gpiochip4 (RP1) and requests lines.
     */
    bool init();

    /** @brief Green LED on, Red/Buzzer off. */
    void granted();

    /** @brief Red LED and Buzzer pulse. */
    void denied();

    void setRedLed(bool state);
    void setGreenLed(bool state);
    void setBuzzer(bool state);

private:
    /** * FIX: Reordered to match logical initialization flow.
     * We define the chip name and offsets first.
     */
    const std::string chip_path = "/dev/gpiochip4"; 
    
    unsigned int red_offset = 17;   // BCM 17
    unsigned int green_offset = 27; // BCM 27
    unsigned int buzzer_offset = 22; // BCM 22

    /** * libgpiod v2 objects. 
     * Note: In modern libgpiod, the request object holds the lines.
     */
    gpiod::chip chip;
    gpiod::line_request request;
};