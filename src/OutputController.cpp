#include "OutputController.hpp"
#include <thread>
#include <chrono>

/**
 * @brief Initializes the GPIO chip and claims output lines.
 */
bool OutputController::init()
{
    try {
        // gpiochip4 is the standard for RP1 on Raspberry Pi 5
        chip = gpiod::chip(chip_path);

        gpiod::line_settings settings;
        settings.set_direction(gpiod::line_direction::OUTPUT);
        settings.set_output_value(gpiod::line_value::INACTIVE);

        gpiod::line_config cfg;
        // Efficiently request all three lines in a single bulk operation
        cfg.add_line_settings({red_offset, green_offset, buzzer_offset}, settings);

        // Build and execute the request
        request = chip.prepare_config()
                      .set_line_config(cfg)
                      .set_consumer("smart_door_out")
                      .request();

        return true;
    } catch (...) {
        // Safe fail-state for Raspberry Pi hardware access
        return false;
    }
}

void OutputController::setRedLed(bool state)
{
    request.set_value(red_offset, state ? gpiod::line_value::ACTIVE : gpiod::line_value::INACTIVE);
}

void OutputController::setGreenLed(bool state)
{
    request.set_value(green_offset, state ? gpiod::line_value::ACTIVE : gpiod::line_value::INACTIVE);
}

void OutputController::setBuzzer(bool state)
{
    request.set_value(buzzer_offset, state ? gpiod::line_value::ACTIVE : gpiod::line_value::INACTIVE);
}

/**
 * @brief "Access Granted" signal.
 * Added a sleep so the green light is actually visible.
 */
void OutputController::granted()
{
    setGreenLed(true);
    setRedLed(false);
    setBuzzer(false);
    
    // Allow the user to see the green light for 2 seconds
    std::this_thread::sleep_for(std::chrono::seconds(2));
    setGreenLed(false);
}

/**
 * @brief "Access Denied" signal.
 * Added a pulse loop for better user feedback.
 */
void OutputController::denied()
{
    setGreenLed(false);
    
    // Pulse red and buzzer 3 times
    for (int i = 0; i < 3; ++i) {
        setRedLed(true);
        setBuzzer(true);
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        setRedLed(false);
        setBuzzer(false);
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }
}