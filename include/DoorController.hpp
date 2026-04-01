#ifndef DOOR_CONTROLLER_HPP
#define DOOR_CONTROLLER_HPP

#include <gpiod.hpp>
#include <nfc/nfc.h>
#include <string>

class DoorController {
public:
    DoorController(unsigned int reedPin);
    ~DoorController();

    bool initialize();
    void checkDoorStatus();
    void processNFC();

private:
    // GPIO Members
    unsigned int reedPin;
    int lastDoorState = -1;
    gpiod::chip chip;
    gpiod::line line;

    // NFC Members
    nfc_context *context = nullptr;
    nfc_device *pnd = nullptr;
};

#endif