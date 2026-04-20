#include "OutputController.hpp"

/**
 * @brief Initializes the GPIO chip and claims output lines.
 * @details Targets /dev/gpiochip4 for the Raspberry Pi 5 (RP1). 
 * Requests Red, Green, and Buzzer lines in a single bulk request for atomicity.
 * @return true if hardware was successfully claimed, false on exception (e.g., chip busy).
 */
bool OutputController::init()
{
    try {
        // gpiochip4 is the standard for RP1 on Raspberry Pi 5
        chip = std::make_shared<gpiod::chip>("/dev/gpiochip4");

        gpiod::line_settings settings;
        settings.set_direction(gpiod::line::direction::OUTPUT);
        settings.set_output_value(gpiod::line::value::INACTIVE);

        gpiod::line_config cfg;
        // Efficiently request all three lines in a single bulk operation
        cfg.add_line_settings({red_offset, green_offset, buzzer_offset}, settings);

        auto builder = chip->prepare_request();
        builder.set_line_config(cfg);
        builder.set_consumer("smart_door_out");

        req = std::make_shared<gpiod::line_request>(builder.do_request());
        return true;
    } catch (...) {
        // Safe fail-state: prevent system crash if hardware is inaccessible
        return false;
    }
}

/**
 * @brief Controls the state of the Red LED.
 * @param state true for ACTIVE (ON), false for INACTIVE (OFF).
 */
void OutputController::setRedLed(bool state)
{
    if (req) {
        req->set_value(red_offset, state ? gpiod::line::value::ACTIVE : gpiod::line::value::INACTIVE);
    }
}

/**
 * @brief Controls the state of the Green LED.
 * @param state true for ACTIVE (ON), false for INACTIVE (OFF).
 */
void OutputController::setGreenLed(bool state)
{
    if (req) {
        req->set_value(green_offset, state ? gpiod::line::value::ACTIVE : gpiod::line::value::INACTIVE);
    }
}

/**
 * @brief Controls the state of the Piezo Buzzer.
 * @param state true for ACTIVE (ON), false for INACTIVE (OFF).
 */
void OutputController::setBuzzer(bool state)
{
    if (req) {
        req->set_value(buzzer_offset, state ? gpiod::line::value::ACTIVE : gpiod::line::value::INACTIVE);
    }
}

/**
 * @brief Visual and audible "Access Granted" signal.
 * * Turns Green LED ON, Red LED and Buzzer OFF.
 */
void OutputController::granted()
{
    if (req) {
        setGreenLed(true);
        setRedLed(false);
        setBuzzer(false);
    }
}

/**
 * @brief Visual and audible "Access Denied" signal.
 * * Turns Red LED and Buzzer ON, Green LED OFF.
 */
void OutputController::denied()
{
    if (req) {
        setRedLed(true);
        setGreenLed(false);
        setBuzzer(true);
    }
}