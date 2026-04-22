#include "OutputController.hpp"
#include <thread>
#include <iostream>

/**
 * @brief Claims the GPIO hardware and configures the output pins.
 */
bool OutputController::init()
{
    std::lock_guard<std::mutex> lock(mtx_);
    try {
        chip = std::make_shared<gpiod::chip>("/dev/gpiochip4");

        gpiod::line_settings settings;
        settings.set_direction(gpiod::line::direction::OUTPUT);
        settings.set_output_value(gpiod::line::value::INACTIVE);

        gpiod::line_config cfg;
        cfg.add_line_settings({red_offset, green_offset, buzzer_offset}, settings);

        auto builder = chip->prepare_request();
        builder.set_line_config(cfg);
        builder.set_consumer("smart_door_out");

        req = std::make_shared<gpiod::line_request>(builder.do_request());
        return true;
        
    } catch (const std::exception& e) {
        std::cerr << "GPIO Hardware Error: " << e.what() << std::endl;
        return false;
    }
}

void OutputController::setRedLed(bool state)
{
    std::lock_guard<std::mutex> lock(mtx_);
    if (req) {
        req->set_value(red_offset, state ? gpiod::line::value::ACTIVE : gpiod::line::value::INACTIVE);
    }
}

void OutputController::setGreenLed(bool state)
{
    std::lock_guard<std::mutex> lock(mtx_);
    if (req) {
        req->set_value(green_offset, state ? gpiod::line::value::ACTIVE : gpiod::line::value::INACTIVE);
    }
}

void OutputController::setBuzzer(bool state)
{
    std::lock_guard<std::mutex> lock(mtx_);
    if (req) {
        req->set_value(buzzer_offset, state ? gpiod::line::value::ACTIVE : gpiod::line::value::INACTIVE);
    }
}

void OutputController::granted() {
    // Just use the internal thread-safe setters you already have
    setGreenLed(true);
    setRedLed(false);
    setBuzzer(true);

    std::thread([this]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        setBuzzer(false);
    }).detach();
}

void OutputController::denied()
{
    std::thread([this]() {
        setRedLed(true);
        setGreenLed(false);
        setBuzzer(true);
        
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        
        setBuzzer(false);
        setRedLed(false);
    }).detach(); 
}
