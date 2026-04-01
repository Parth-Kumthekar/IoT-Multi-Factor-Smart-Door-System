#include "DoorController.hpp"
#include "OutputHandler.hpp"
#include <iostream>
#include <vector>

// Add a list of authorized UIDs for the demo
std::vector<std::string> authorizedUIDs = {"046732ca", "a1b2c3d4"}; 

void DoorController::processNFC(OutputHandler& outputs) {
    nfc_target nt;
    const nfc_modulation nm = { .nmt = NMT_ISO14443A, .nbr = NBR_106 };

    if (nfc_initiator_poll_target(pnd, &nm, 1, 1, 2, &nt) > 0) {
        // Convert hex UID to string for comparison
        std::string uid = "";
        for (size_t i = 0; i < nt.nti.nai.szUidLen; i++) {
            char buf[3];
            sprintf(buf, "%02x", nt.nti.nai.abtUid[i]);
            uid += buf;
        }

        std::cout << "[NFC] Scanned: " << uid << std::endl;

        // Check if UID is authorized
        bool found = false;
        for(const auto& id : authorizedUIDs) {
            if(id == uid) { found = true; break; }
        }

        if (found) {
            outputs.setAccessGranted();
            // Wait for user to open and close door via Reed sensor
            std::cout << "Waiting for door to close..." << std::endl;
            std::this_thread::sleep_for(std::chrono::seconds(5)); 
            outputs.lock();
        } else {
            outputs.setAccessDenied();
            outputs.lock();
        }
    }
}