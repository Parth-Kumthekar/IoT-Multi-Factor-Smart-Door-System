#ifndef DOOR_CONTROLLER_HPP
#define DOOR_CONTROLLER_HPP

#include <gpiod.hpp>
#include <nfc/nfc.h>
#include <string>
#include <optional> // Required for Pi 5 / libgpiod v2

class DoorController {
public:
    DoorController(unsigned int reedPin);
    ~DoorController();

    bool initialize();
    void checkDoorStatus();
    void processNFC();

private:
    bool isDoorOpen();

    // GPIO Members
    unsigned int reedPinNum;
    int lastDoorState = -1;
    
    // Wrap in optional to bypass the private constructor error
    std::optional<gpiod::line_request> reed_request;

    // NFC Members
    nfc_context *context = nullptr;
    nfc_device *pnd = nullptr;
};

#endif