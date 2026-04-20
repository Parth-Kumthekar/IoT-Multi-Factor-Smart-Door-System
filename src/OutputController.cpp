#include "OutputController.hpp"

/**
 * @brief Claims the GPIO hardware and configures the output pins.
 * @details Specifically targets `/dev/gpiochip4` (the RP1 peripheral on Raspberry Pi 5). 
 * It uses a bulk request to configure the Red LED, Green LED, and Buzzer offsets 
 * simultaneously, setting them all as OUTPUT with an initial INACTIVE state.
 * @return true if the chip was opened and lines were successfully requested.
 * @return false if the chip is busy or the process lacks necessary GPIO permissions.
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
        // Request all three lines at once
        cfg.add_line_settings({red_offset, green_offset, buzzer_offset}, settings);

        auto builder = chip->prepare_request();
        builder.set_line_config(cfg);
        builder.set_consumer("smart_door_out");

        req = std::make_shared<gpiod::line_request>(builder.do_request());
        return true;
    } catch (...) {
        return false;
    }
}

/**
 * @brief Sets the logical state of the Red LED.
 * @param state true for ACTIVE (On), false for INACTIVE (Off).
 */
void OutputController::setRedLed(bool state)
{
    if (req) {
        req->set_value(red_offset, state ? gpiod::line::value::ACTIVE : gpiod::line::value::INACTIVE);
    }
}

/**
 * @brief Sets the logical state of the Green LED.
 * @param state true for ACTIVE (On), false for INACTIVE (Off).
 */
void OutputController::setGreenLed(bool state)
{
    if (req) {
        req->set_value(green_offset, state ? gpiod::line::value::ACTIVE : gpiod::line::value::INACTIVE);
    }
}

/**
 * @brief Sets the logical state of the Buzzer.
 * @param state true for ACTIVE (On), false for INACTIVE (Off).
 */
void OutputController::setBuzzer(bool state)
{
    if (req) {
        req->set_value(buzzer_offset, state ? gpiod::line::value::ACTIVE : gpiod::line::value::INACTIVE);
    }
}

/**
 * @brief Executes the "Access Granted" signaling pattern.
 * @details Synchronously updates the state of all three outputs: Green LED ON, 
 * Red LED OFF, and Buzzer OFF.
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
 * @brief Executes the "Access Denied" signaling pattern.
 * @details Synchronously updates the state of all three outputs: Red LED ON, 
 * Buzzer ON, and Green LED OFF.
 */
void OutputController::denied()
{
    if (req) {
        setRedLed(true);
        setGreenLed(false);
        setBuzzer(true);
    }
}
