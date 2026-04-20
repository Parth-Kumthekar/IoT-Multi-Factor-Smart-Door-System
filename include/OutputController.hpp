#pragma once
#include <gpiod.hpp>
#include <memory>
#include <vector>

/**
 * @class OutputController
 * @brief Manages the physical feedback components (LEDs and Buzzer).
 * * This class provides a high-level interface to drive the system's actuators.
 * It abstracts the specific GPIO offsets and line requests, allowing the FSM
 * to trigger "Granted" or "Denied" feedback without needing to manage 
 * individual hardware pins.
 */
class OutputController {
public:
    /**
     * @brief Initializes the GPIO chip and requests output lines.
     * * Specifically targets the RP1 chip (gpiochip4) for Raspberry Pi 5.
     * @return true if the chip was opened and lines were successfully claimed.
     */
    bool init();

    /**
     * @brief Visual and audible feedback for authorized access.
     * * Synchronously sets the Green LED to active and ensures Red/Buzzer are inactive.
     */
    void granted();

    /**
     * @brief Visual and audible feedback for unauthorized access or security breaches.
     * * Synchronously activates the Red LED and Buzzer to alert the user/environment.
     */
    void denied();

    /**
     * @brief Manual control for the Red Status LED.
     * @param state true to enable current flow to the LED.
     */
    void setRedLed(bool state);

    /**
     * @brief Manual control for the Green Status LED.
     * @param state true to enable current flow to the LED.
     */
    void setGreenLed(bool state);

    /**
     * @brief Manual control for the piezoelectric Buzzer.
     * @param state true to activate the audible alarm.
     */
    void setBuzzer(bool state);

private:
    /** @brief Shared pointer to the GPIO chip resource for safe lifecycle management. */
    std::shared_ptr<gpiod::chip> chip;

    /** @brief Shared pointer to the bulk line request managing the three outputs. */
    std::shared_ptr<gpiod::line_request> req;

    /** @brief GPIO offset for the Red LED (BCM 17 / Physical Pin 11). */
    unsigned int red_offset = 17;

    /** @brief GPIO offset for the Green LED (BCM 27 / Physical Pin 13). */
    unsigned int green_offset = 27;

    /** @brief GPIO offset for the Buzzer (BCM 22 / Physical Pin 15). */
    unsigned int buzzer_offset = 22;
};