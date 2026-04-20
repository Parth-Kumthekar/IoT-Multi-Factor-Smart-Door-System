#pragma once
#include <gpiod.hpp>
#include <memory>
#include <vector>

/**
 * @class OutputController
 * @brief Manages physical output signals including status LEDs and the audible buzzer.
 * * This class abstracts the GPIO operations for the signaling hardware. It provides
 * both high-level semantic commands (like granted/denied) and direct low-level 
 * control over specific GPIO offsets.
 */
class OutputController {
public:
    /**
     * @brief Initializes gpiochip4 and requests lines for Red, Green, and Buzzer.
     * @details Configures the specified GPIO offsets as outputs.
     * @return true if hardware was successfully claimed, false otherwise.
     */
    bool init();

    /**
     * @brief High-level command for valid access: Green LED ON, others OFF.
     * @details Typically used to indicate a successful UID verification.
     */
    void granted();

    /**
     * @brief High-level command for invalid access: Red LED + Buzzer ON.
     * @details Typically used to indicate a failed verification or an unauthorized entry.
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
    /// Smart pointer to the GPIO chip device (gpiochip4 for Raspberry Pi 5).
    std::shared_ptr<gpiod::chip> chip;

    /// Smart pointer to the bulk line request for the three output pins.
    std::shared_ptr<gpiod::line_request> req;

    // GPIO numbers for Raspberry Pi 5 Header (RP1 chip)
    // These are GPIO offsets, not physical pin numbers

    /// GPIO offset for the Red LED (Physical Pin 11).
    unsigned int red_offset = 17;   
    
    /// GPIO offset for the Green LED (Physical Pin 13).
    unsigned int green_offset = 27; 
    
    /// GPIO offset for the Buzzer (Physical Pin 15).
    unsigned int buzzer_offset = 22; 
};
