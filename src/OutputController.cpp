#include "OutputController.hpp"
bool OutputController::init()
{
    // CRITICAL: Change 0 to 4 for Raspberry Pi 5
    chip = std::make_shared<gpiod::chip>("/dev/gpiochip4");

    gpiod::line_settings settings;
    settings.set_direction(gpiod::line::direction::OUTPUT);
    settings.set_output_value(gpiod::line::value::INACTIVE);

    gpiod::line_config cfg;
    // Ensure these pin numbers (17, 27, 22) match your physical wiring
    cfg.add_line_settings({red, green, buzzer}, settings);

    auto builder = chip->prepare_request();
    builder.set_line_config(cfg);
    builder.set_consumer("smart_door_out");

    req = std::make_shared<gpiod::line_request>(builder.do_request());
    return true;
}

void OutputController::granted()
{
    req->set_value(green, gpiod::line::value::ACTIVE);
    req->set_value(red, gpiod::line::value::INACTIVE);
    req->set_value(buzzer, gpiod::line::value::INACTIVE);
}

void OutputController::denied()
{
    req->set_value(red, gpiod::line::value::ACTIVE);
    req->set_value(green, gpiod::line::value::INACTIVE);
    req->set_value(buzzer, gpiod::line::value::ACTIVE);
}