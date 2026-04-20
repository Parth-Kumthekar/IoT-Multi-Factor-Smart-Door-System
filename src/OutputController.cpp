#include "OutputController.hpp"

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

void OutputController::setRedLed(bool state)
{
    if (req) {
        req->set_value(red_offset, state ? gpiod::line::value::ACTIVE : gpiod::line::value::INACTIVE);
    }
}

void OutputController::setGreenLed(bool state)
{
    if (req) {
        req->set_value(green_offset, state ? gpiod::line::value::ACTIVE : gpiod::line::value::INACTIVE);
    }
}

void OutputController::setBuzzer(bool state)
{
    if (req) {
        req->set_value(buzzer_offset, state ? gpiod::line::value::ACTIVE : gpiod::line::value::INACTIVE);
    }
}

void OutputController::granted()
{
    if (req) {
        setGreenLed(true);
        setRedLed(false);
        setBuzzer(false);
    }
}

void OutputController::denied()
{
    if (req) {
        setRedLed(true);
        setGreenLed(false);
        setBuzzer(true);
    }
}
