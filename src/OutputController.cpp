#include "OutputController.hpp"

bool OutputController::init() {
    chip = std::make_shared<gpiod::chip>("/dev/gpiochip0");

    gpiod::line_config config;
    config.add_line_settings(
        {green, red, buzzer},
        gpiod::line_settings()
            .set_direction(gpiod::line::direction::OUTPUT)
            .set_output_value(gpiod::line::value::INACTIVE)
    );

    auto builder = chip->prepare_request();
    builder.set_consumer("output_ctrl");
    builder.set_line_config(config);

    req = std::make_shared<gpiod::line_request>(builder.do_request());

    return true;
}

void OutputController::accessGranted() {
    req->set_value(green, gpiod::line::value::ACTIVE);
    req->set_value(red, gpiod::line::value::INACTIVE);
    req->set_value(buzzer, gpiod::line::value::INACTIVE);
}

void OutputController::accessDenied() {
    req->set_value(green, gpiod::line::value::INACTIVE);
    req->set_value(red, gpiod::line::value::ACTIVE);
    req->set_value(buzzer, gpiod::line::value::ACTIVE);
}