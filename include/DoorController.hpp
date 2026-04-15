#pragma once
#include "gpiopin.hpp"
#include "AccessController.hpp"
#include "OutputController.hpp"
#include "NFCReader.hpp"
#include <functional>

class DoorController {
public:
    DoorController(int reedPin, AccessController& ac, OutputController& oc, NFCReader& nfc);
    void initialize();
    void stop();

private:
    GPIOPin reedSwitch;
    AccessController& accessController;
    OutputController& outputController;
    NFCReader& nfcReader;

    void onDoorOpen();
};