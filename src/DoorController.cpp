#include "DoorController.hpp"
#include <iostream>
#include <iomanip>
#include <sstream>

// Constructor: Initializes the BCM pin number for the Reed Switch
DoorController::DoorController(unsigned int reedPin) : reedPinNum(reedPin) {}

// Destructor: Ensures NFC hardware is released properly
DoorController::~DoorController() {
    if (pnd) nfc_close(pnd);
    if (context) nfc_exit(context);
}

bool DoorController::initialize() {
    try {
        // 1. Initialize GPIO for Reed Switch (Pi 5 chip 4)
        gpiod::chip chip("/dev/gpiochip4");

        // Define settings for Input with a Pull-Up resistor
        auto settings = gpiod::line_settings()
            .set_direction(gpiod::line::direction::INPUT)
            .set_bias(gpiod::line::bias::PULL_UP);

        // Map settings to the Reed Pin
        auto line_cfg = gpiod::line_config();
        line_cfg.add_line_settings({reedPinNum}, settings);

        // Submit request and move it into the class member (std::optional)
        reed_request = std::move(chip.request_lines(line_cfg));

        // 2. Initialize libnfc
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

    // Read the current physical state of the magnet
    // With PULL_UP: 1 (ACTIVE) means the magnet is AWAY (Door Open)
    bool isOpen = (reed_request->get_value(reedPinNum) == gpiod::line::value::ACTIVE);

    // Only print when the state changes to avoid flooding the terminal
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

    // Non-blocking poll for a single target
    // We pass 0 for timeout to make it a quick check inside the main loop
    if (nfc_initiator_select_passive_target(pnd, nm, NULL, 0, &nt) > 0) {
        std::stringstream ss;
        for (size_t i = 0; i < nt.nti.nai.szUidLen; i++) {
            ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(nt.nti.nai.abtUid[i]);
        }
        return ss.str();
    }
    
    return "";
}