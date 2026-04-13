#include "DoorController.hpp"
#include <iostream>

DoorController::DoorController(int reedPin, AccessController& ac, OutputController& oc, NFCReader& nfc)
    : reedSwitch(reedPin, false), accessController(ac), outputController(oc), nfcReader(nfc) {}

void DoorController::initialize() {
    reedSwitch.registerCallback([this]() { onDoorOpen(); }, 50);
    reedSwitch.start();
}

void DoorController::stop() {
    reedSwitch.stop();
}

void DoorController::onDoorOpen() {
    std::cout << "Door opened! Waiting for NFC..." << std::endl;
    std::string uid = nfcReader.readUID();
    if (accessController.isAuthorized(uid)) {
        std::cout << "Access Granted" << std::endl;
        outputController.setAccessGranted();
    } else {
        std::cout << "Access Denied" << std::endl;
        outputController.setAccessDenied();
    }
}