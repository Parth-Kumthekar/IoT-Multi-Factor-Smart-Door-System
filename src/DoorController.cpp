#include "DoorController.hpp"
#include <iostream>
#include <iomanip>

DoorController::DoorController(unsigned int pin) : reedPin(pin) {}

DoorController::~DoorController() {
    if (pnd) nfc_close(pnd);
    if (context) nfc_exit(context);
}

bool DoorController::initialize() {
    // Initialize GPIO (Raspberry Pi 5 RP1)
    try {
        chip = gpiod::make_chip("gpiochip4");
        line = chip.get_line(reedPin);
        line.request({
            "DoorSensor", 
            gpiod::line_request::DIRECTION_INPUT, 
            gpiod::line_request::FLAG_BIAS_PULL_UP
        }, 0);
    } catch (const std::exception& e) {
        std::cerr << "GPIO Init Error: " << e.what() << std::endl;
        return false;
    }

    // Initialize NFC
    nfc_init(&context);
    if (!context) return false;

    pnd = nfc_open(context, NULL);
    if (!pnd) {
        std::cerr << "ReadPi NFC not detected!" << std::endl;
        return false;
    }

    return (nfc_initiator_init(pnd) >= 0);
}

void DoorController::checkDoorStatus() {
    int currentState = line.get_value();
    if (currentState != lastDoorState) {
        std::cout << "[SENSOR] Door is now: " 
                  << (currentState == 0 ? "CLOSED" : "OPEN") << std::endl;
        lastDoorState = currentState;
    }
}

void DoorController::processNFC() {
    nfc_target nt;
    const nfc_modulation nm = { .nmt = NMT_ISO14443A, .nbr = NBR_106 };

    // Poll for 100ms so we don't block the door sensor check
    if (nfc_initiator_poll_target(pnd, &nm, 1, 1, 2, &nt) > 0) {
        std::cout << "[NFC] Access Attempt - UID: ";
        for (size_t i = 0; i < nt.nti.nai.szUidLen; i++) {
            std::cout << std::hex << std::setw(2) << std::setfill('0') 
                      << (int)nt.nti.nai.abtUid[i];
        }
        std::cout << std::dec << std::endl;
    }
}