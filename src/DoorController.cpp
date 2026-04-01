#include "DoorController.hpp"
#include <iostream>
#include <iomanip>
#include <sstream>

DoorController::DoorController(unsigned int reedPin) : reedPinNum(reedPin) {}

DoorController::~DoorController() {
    if (pnd) nfc_close(pnd);
    if (context) nfc_exit(context);
}

bool DoorController::initialize() {
    try {
        gpiod::chip chip("/dev/gpiochip4");
        reed_request = std::move(chip.prepare_config()
            .add_line_settings(
                reedPinNum,
                gpiod::line_settings()
                    .set_direction(gpiod::line::direction::INPUT)
                    .set_bias(gpiod::line::bias::PULL_UP)
            )
            .request());

        nfc_init(&context);
        if (!context) return false;
        pnd = nfc_open(context, nullptr);
        if (!pnd) return false;
        nfc_initiator_init(pnd);

        return true;
    } catch (const std::exception& e) {
        std::cerr << "Door Init Error: " << e.what() << std::endl;
        return false;
    }
}

void DoorController::checkDoorStatus() {
    if (!reed_request) return;
    bool isOpen = (reed_request->get_value(reedPinNum) == gpiod::line::value::ACTIVE);
    if ((int)isOpen != lastDoorState) {
        std::cout << "Door is " << (isOpen ? "OPEN" : "CLOSED") << std::endl;
        lastDoorState = (int)isOpen;
    }
}

std::string DoorController::scanNFC() {
    nfc_target nt;
    const nfc_modulation nm = {.nmt = NMT_ISO14443A, .nbr = NBR_106};
    
    // Non-blocking poll
    if (nfc_initiator_select_passive_target(pnd, nm, NULL, 0, &nt) > 0) {
        std::stringstream ss;
        for (size_t i = 0; i < nt.nti.nai.szUidLen; i++) {
            ss << std::hex << std::setw(2) << std::setfill('0') << (int)nt.nti.nai.abtUid[i];
        }
        return ss.str();
    }
    return "";
}