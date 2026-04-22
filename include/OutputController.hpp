#pragma once
#include <gpiod.hpp>
#include <memory>
#include <vector>
#include <mutex> // Added for thread safety

/**
 * @class OutputController
 * @brief Manages physical output signals including status LEDs and the audible buzzer.
 * @details This class is thread-safe, allowing multiple system components (FSM, Timer, Camera)
 * to update hardware states simultaneously without race conditions.
 */
class OutputController {
public:
    /**
     * @brief Initializes the GPIO chip and requests lines for Red, Green, and Buzzer.
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
    /// Protects GPIO line requests from concurrent access across threads.
    std::mutex mtx_;

    /// Smart pointer to the GPIO chip device (e.g., gpiochip4 for Raspberry Pi 5).
    std::shared_ptr<gpiod::chip> chip;

    /// Smart pointer to the bulk line request for the three output pins.
    std::shared_ptr<gpiod::line_request> req;

    // GPIO offsets for Raspberry Pi 5 (RP1 chip)
    
    /// GPIO offset for the Red LED.
    unsigned int red_offset = 23;   
    
    /// GPIO offset for the Green LED.
    unsigned int green_offset = 27; 
    
    /// GPIO offset for the Buzzer.
    unsigned int buzzer_offset = 22;
};
