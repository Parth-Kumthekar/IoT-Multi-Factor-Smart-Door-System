#ifndef OUTPUT_HANDLER_HPP
#define OUTPUT_HANDLER_HPP

#include <gpiod.hpp>
#include <thread>
#include <chrono>
#include <optional>

class OutputHandler {
public:
    OutputHandler(unsigned int relayPin, unsigned int greenPin, unsigned int redPin, unsigned int buzzerPin);

    bool init();
    void setAccessGranted();
    void setAccessDenied();
    void lock();

private:
    void beep(int duration_ms);

    unsigned int rp, gp, rdp, bp;
    std::optional<gpiod::line_request> line_request; 
};

#endif