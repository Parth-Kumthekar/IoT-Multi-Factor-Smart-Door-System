#include "GPIOPin.hpp"
#include <iostream>

#define TIMEOUT 5000

GPIOPin::GPIOPin() {}

GPIOPin::~GPIOPin() {
    stop();
}

void GPIOPin::registerCallback(Callback cb) {
    callback = cb;
}

void GPIOPin::start(int pin, int chipNo) {
    std::string chipPath = "/dev/gpiochip" + std::to_string(chipNo);

    chip = std::make_shared<gpiod::chip>(chipPath);

    gpiod::line_config config;
    config.add_line_settings(
        pin,
        gpiod::line_settings()
            .set_direction(gpiod::line::direction::INPUT)
            .set_edge_detection(gpiod::line::edge::BOTH)
    );

    auto builder = chip->prepare_request();
    builder.set_consumer("gpio_handler");
    builder.set_line_config(config);

    request = std::make_shared<gpiod::line_request>(builder.do_request());

    running = true;
    thr = std::thread(&GPIOPin::worker, this);
}

void GPIOPin::worker() {
    while (running) {
        if (request->wait_edge_events(std::chrono::milliseconds(TIMEOUT))) {
            gpiod::edge_event_buffer buffer;
            request->read_edge_events(buffer, 1);
            gpioEvent(buffer.get_event(0));
        }
    }
}

void GPIOPin::gpioEvent(const gpiod::edge_event& ev) {
    if (callback) callback(ev);
}

void GPIOPin::stop() {
    running = false;
    if (thr.joinable()) thr.join();

    if (request) request->release();
}