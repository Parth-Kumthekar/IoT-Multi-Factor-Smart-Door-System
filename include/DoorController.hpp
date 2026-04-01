#ifndef DOOR_CONTROLLER_HPP
#define DOOR_CONTROLLER_HPP

#include <gpiod.hpp>
#include <nfc/nfc.h>
#include <string>
#include <memory>

class DoorController {
public:
    DoorController(unsigned int reedPin);
    ~DoorController();

    bool initialize();
    void checkDoorStatus();
    void processNFC();

private:
    // GPIO Members for Pi 5 (v2.x)
    unsigned int reedPinNum;
    int lastDoorState = -1;
    
    // In v2.x, we keep the line_request rather than 'line' or 'chip'
    gpiod::line_request reed_request;

    // NFC Members
    nfc_context *context = nullptr;
    nfc_device *pnd = nullptr;

    // Helper to read the physical pin
    bool isDoorOpen();
};

#endif