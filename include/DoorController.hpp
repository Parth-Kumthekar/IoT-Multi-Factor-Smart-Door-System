#ifndef DOOR_CONTROLLER_HPP
#define DOOR_CONTROLLER_HPP

#include <nfc/nfc.h>
#include <string>
#include <optional>
#include <vector>
#include <gpiod.hpp>

class DoorController {
public:
    DoorController(unsigned int reedPin);
    ~DoorController();

    bool initialize();
    void checkDoorStatus();
    std::string scanNFC(); // Returns UID string if found

private:
    unsigned int reedPinNum;
    int lastDoorState = -1;
    std::optional<gpiod::line_request> reed_request;

    nfc_context *context = nullptr;
    nfc_device *pnd = nullptr;
};

#endif