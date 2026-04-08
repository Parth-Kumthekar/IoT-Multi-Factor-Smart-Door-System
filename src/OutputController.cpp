#include "OutputController.hpp"
#include <thread>
#include <chrono>

std::shared_ptr<gpiod::line_request>
OutputController::createOutput(int pin) {
    gpiod::line_config cfg;

    cfg.add_line_settings(
        pin,
        gpiod::line_settings()
            .set_direction(gpiod::line::direction::OUTPUT)
            .set_output_value(gpiod::line::value::INACTIVE)
    );

    auto builder = chip->prepare_request();
    builder.set_consumer("output");
    builder.set_line_config(cfg);

    return std::make_shared<gpiod::line_request>(builder.do_request());
}

void OutputController::init() {
    chip = std::make_shared<gpiod::chip>("/dev/gpiochip0");

    green  = createOutput(22);
    red    = createOutput(23);
    buzzer = createOutput(24);
}

void OutputController::set(std::shared_ptr<gpiod::line_request> line, bool val) {
    line->set_value(0, val ? gpiod::line::value::ACTIVE
                           : gpiod::line::value::INACTIVE);
}

void OutputController::accessGranted() {
    set(green, true);
    set(red, false);
    set(buzzer, false);
}

void OutputController::accessDenied() {
    set(green, false);
    set(red, true);
    set(buzzer, true);

    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    set(buzzer, false);
}