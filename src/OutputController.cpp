#include "OutputController.hpp"

bool OutputController::init()
{
    chip = std::make_shared<gpiod::chip>("/dev/gpiochip0");

    gpiod::line_settings settings;
    settings.set_direction(gpiod::line::direction::OUTPUT);
    settings.set_output_value(gpiod::line::value::INACTIVE);

    gpiod::line_config cfg;
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