#include "gpiopin.hpp"
#include <chrono>
#include <iostream>

void GPIOPin::start(int pin, int chipNo)
{
    pinNum = pin;

    std::string chipPath = "/dev/gpiochip" + std::to_string(chipNo);
    chip = std::make_shared<gpiod::chip>(chipPath);

    gpiod::line_settings settings;
    settings.set_direction(gpiod::line::direction::INPUT);
    settings.set_edge_detection(gpiod::line::edge::BOTH);

    gpiod::line_config cfg;
    cfg.add_line_settings(pin, settings);

    auto builder = chip->prepare_request();
    builder.set_line_config(cfg);
    builder.set_consumer("smart_door");

    request = std::make_shared<gpiod::line_request>(builder.do_request());

    running = true;
    thr = std::thread(&GPIOPin::worker, this);
}

void GPIOPin::worker()
{
    while (running)
    {
        if (request->wait_edge_events(std::chrono::milliseconds(500)))
        {
            gpiod::edge_event_buffer buf;
            request->read_edge_events(buf);

            for (unsigned int i = 0; i < buf.num_events(); i++)
            {
                auto event = buf.get_event(i);

                int val =
                    (event.type() ==
                     gpiod::edge_event::event_type::RISING_EDGE) ? 1 : 0;

                if (callback)
                    callback(val);
            }
        }
    }
}

void GPIOPin::stop()
{
    running = false;

    if (thr.joinable())
        thr.join();
}