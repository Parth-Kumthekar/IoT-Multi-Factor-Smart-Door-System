#include "OutputController.hpp"

bool OutputController::init()
{
    try {
        // CRITICAL: Change 0 to 4 for Raspberry Pi 5
        chip = std::make_shared<gpiod::chip>("/dev/gpiochip4");

        gpiod::line_settings settings;
        settings.set_direction(gpiod::line::direction::OUTPUT);
        settings.set_output_value(gpiod::line::value::INACTIVE);

        gpiod::line_config cfg;
        // Pin numbers (17, 27, 22) map to Physical Pins (11, 13, 15)
        cfg.add_line_settings({(unsigned int)red, (unsigned int)green, (unsigned int)buzzer}, settings);

        auto builder = chip->prepare_request();
        builder.set_line_config(cfg);
        builder.set_consumer("smart_door_out");

        req = std::make_shared<gpiod::line_request>(builder.do_request());
        return true;
    } catch (...) {
        return false;
    }
}

// Add this function to drive the LED directly from the Reed Switch status
void OutputController::setRedLed(bool state)
{
    if (req) {
        req->set_value((unsigned int)red, state ? gpiod::line::value::ACTIVE : gpiod::line::value::INACTIVE);
    }
}

void OutputController::granted()
{
    req->set_value((unsigned int)green, gpiod::line::value::ACTIVE);
    req->set_value((unsigned int)red, gpiod::line::value::INACTIVE);
    req->set_value((unsigned int)buzzer, gpiod::line::value::INACTIVE);
}

void OutputController::denied()
{
    req->set_value((unsigned int)red, gpiod::line::value::ACTIVE);
    req->set_value((unsigned int)green, gpiod::line::value::INACTIVE);
    req->set_value((unsigned int)buzzer, gpiod::line::value::ACTIVE);
}