#include "DoorController.hpp"
#include <iostream>
#include <iomanip>
#include <algorithm>

// Authorized UID - Replace with your actual card UID after running nfc-poll
const std::string MASTER_CARD_UID = "046732ca"; 

DoorController::DoorController(unsigned int reedPin) : reedPinNum(reedPin) {}

DoorController::~DoorController() {
    if (pnd) nfc_close(pnd);
    if (context) nfc_exit(context);
}

bool DoorController::initialize() {
    try {
        // 1. Initialize GPIO for Reed Switch (Pi 5 uses chip 4)
        auto chip = gpiod::make_chip("/dev/gpiochip4");
        
        reed_request = std::move(chip.prepare_config()
            .add_line_settings(
                reedPinNum,
                gpiod::line_settings()
                    .set_direction(gpiod::line_config::direction::INPUT)
                    .set_bias(gpiod::line_config::bias::PULL_UP)
            )
            .request());

        // 2. Initialize libnfc
        nfc_init(&context);
        if (context == nullptr) {
            std::cerr << "Unable to init libnfc (install libnfc-bin?)" << std::endl;
            return false;
        }
        
        pnd = nfc_open(context, nullptr);
        if (pnd == nullptr) {
            std::cerr << "Unable to open NFC device. Check I2C wiring/permissions." << std::endl;
            return false;
        }

        if (nfc_initiator_init(pnd) < 0) {
            nfc_perror(pnd, "nfc_initiator_init");
            return false;
        }

        std::cout << "Door System Online. Waiting for card..." << std::endl;
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Init Error: " << e.what() << std::endl;
        return false;
    }
}

bool DoorController::isDoorOpen() {
    if (!reed_request) return false;
    auto value = reed_request->get_value(reedPinNum);
    return (value == gpiod::line::value::ACTIVE);
}

void DoorController::checkDoorStatus() {
    bool currentOpen = isDoorOpen();
    if (static_cast<int>(currentOpen) != lastDoorState) {
        std::cout << "--- Door is now " << (currentOpen ? "OPEN" : "CLOSED") << " ---" << std::endl;
        lastDoorState = currentOpen;
    }
}

void DoorController::processNFC() {
    nfc_target nt;
    // Set scanning modulation (ISO14443A is standard for most NFC cards/phones)
    const nfc_modulation nm = {
        .nmt = NMT_ISO14443A,
        .nbr = NBR_106,
    };

    // Non-blocking-ish poll: scan for 1 target
    if (nfc_initiator_select_passive_target(pnd, nm, NULL, 0, &nt) > 0) {
        // Convert the raw bytes of the UID to a Hex string
        std::string scannedUID = "";
        for (size_t i = 0; i < nt.nti.nai.szUidLen; i++) {
            std::stringstream ss;
            ss << std::hex << std::setw(2) << std::setfill('0') << (int)nt.nti.nai.abtUid[i];
            scannedUID += ss.str();
        }

        std::cout << "Scanned UID: " << scannedUID << std::endl;

        // Validation Logic
        if (scannedUID == MASTER_CARD_UID) {
            std::cout << "ACCESS GRANTED: Welcome Home." << std::endl;
            // Here you would call hw.setAccessGranted() if passed in or stored
        } else {
            std::cout << "ACCESS DENIED: Unknown Card." << std::endl;
            // Here you would call hw.setAccessDenied()
        }

        // Small delay to prevent reading the same card 100 times per second
        std::this_thread::sleep_for(std::chrono::seconds(2));
    }
}