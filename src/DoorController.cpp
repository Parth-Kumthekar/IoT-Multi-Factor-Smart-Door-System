#include "DoorController.hpp"
#include <iostream>
#include <iomanip>
#include <sstream>

// Constructor
DoorController::DoorController(unsigned int reedPin) : reedPinNum(reedPin) {}

// Destructor
DoorController::~DoorController() {
    if (pnd) nfc_close(pnd);
    if (context) nfc_exit(context);
}

bool DoorController::initialize() {
    try {
        gpiod::chip chip("/dev/gpiochip4");

        auto settings = gpiod::line_settings()
            .set_direction(gpiod::line::direction::INPUT)
            .set_bias(gpiod::line::bias::PULL_UP);

        auto line_cfg = gpiod::line_config();
        gpiod::line::offsets offsets{reedPinNum};
        line_cfg.add_line_settings(offsets, settings);

        reed_request = chip.request(line_cfg);

        // Initialize NFC
        nfc_init(&context);
        if (!context) {
            std::cerr << "NFC Context Init Failed" << std::endl;
            return false;
        }

        pnd = nfc_open(context, nullptr);
        if (!pnd) {
            std::cerr << "NFC Device Open Failed (Check I2C/Permissions)" << std::endl;
            return false;
        }

        if (nfc_initiator_init(pnd) < 0) {
            nfc_perror(pnd, "nfc_initiator_init");
            return false;
        }

        std::cout << "Door Controller & NFC Initialized (v2.2.1)" << std::endl;
        return true;

    } catch (const std::exception& e) {
        std::cerr << "DoorController Init Error: " << e.what() << std::endl;
        return false;
    }
}

void DoorController::checkDoorStatus() {
    if (!reed_request) return;

    bool isOpen = (reed_request->get_value(reedPinNum) == gpiod::line::value::ACTIVE);

    if (static_cast<int>(isOpen) != lastDoorState) {
        std::cout << "--- Door State Changed: " << (isOpen ? "OPEN" : "CLOSED") << " ---" << std::endl;
        lastDoorState = static_cast<int>(isOpen);
    }
}

std::string DoorController::scanNFC() {
    if (!pnd) return "";

    nfc_target nt;
    const nfc_modulation nm = {
        .nmt = NMT_ISO14443A,
        .nbr = NBR_106,
    };

    if (nfc_initiator_select_passive_target(pnd, nm, NULL, 0, &nt) > 0) {
        std::stringstream ss;
        for (size_t i = 0; i < nt.nti.nai.szUidLen; i++) {
            ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(nt.nti.nai.abtUid[i]);
        }
        return ss.str();
    }

    return "";
}