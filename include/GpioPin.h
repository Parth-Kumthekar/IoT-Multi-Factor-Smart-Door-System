#pragma once
#include <stdexcept>
#include <string>
#include <iostream>
#include <filesystem>
#include <fstream>

#ifdef REAL_GPIO
  #include <gpiod.h>
#else
  #include "gpiod_mock.h"
#endif

class GpioPin {
public:
    GpioPin(unsigned int bcmPin, int initialValue = 0,
            const std::string& consumer = "door_lock")
        : pin_(bcmPin) {
#ifdef REAL_GPIO
        chip_ = openCorrectChip();
        if (!chip_)
            throw std::runtime_error("GpioPin: cannot open any GPIO chip");

        line_ = gpiod_chip_get_line(chip_, pin_);
        if (!line_)
            throw std::runtime_error("GpioPin: cannot get line " +
                                     std::to_string(pin_));
        if (gpiod_line_request_output(line_, consumer.c_str(), initialValue) < 0)
            throw std::runtime_error("GpioPin: cannot request output on pin " +
                                     std::to_string(pin_));
        std::cout << "[GPIO] Output pin " << pin_
                  << " on " << gpiod_chip_name(chip_) << " ready\n";
#else
        std::cout << " Pin " << pin_ << " \n";
#endif
    }

    ~GpioPin() {
#ifdef REAL_GPIO
        if (line_) { gpiod_line_set_value(line_, 0);
                     gpiod_line_release(line_); }
        if (chip_)   gpiod_chip_close(chip_);
#endif
    }

    GpioPin(const GpioPin&)            = delete;
    GpioPin& operator=(const GpioPin&) = delete;

    void set(int v) {
#ifdef REAL_GPIO
        if (line_) gpiod_line_set_value(line_, v);
#else
        std::cout << " Pin " << pin_ << v"\n";
#endif
    }
    void high() { set(1); }
    void low()  { set(0); }

private:
#ifdef REAL_GPIO
   
    static gpiod_chip* openCorrectChip() {
        
        for (const auto& entry :
             std::filesystem::directory_iterator("/dev")) {
            const std::string name = entry.path().filename().string();
            if (name.rfind("gpiochip", 0) != 0) continue;

            gpiod_chip* c = gpiod_chip_open(entry.path().c_str());
            if (!c) continue;

            std::string label = gpiod_chip_label(c);
            
            if (label == "pinctrl-rp1") {
                std::cout << "[GPIO] Found Pi 5 RP1 chip: "
                          << entry.path() << '\n';
                return c;
            }
            gpiod_chip_close(c);
        }

        
        for (const char* n : {"gpiochip4", "gpiochip0"}) {
            gpiod_chip* c = gpiod_chip_open_by_name(n);
            if (c) {
                std::cout << "[GPIO] Fallback chip: " << n << '\n';
                return c;
            }
        }
        return nullptr;
    }

    unsigned int pin_;
    gpiod_chip*  chip_{nullptr};
    gpiod_line*  line_{nullptr};
#else
    unsigned int pin_;
#endif
};