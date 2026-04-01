#include "NFCReader.hpp"
#include <iostream>
#include <iomanip>
#include <sstream>

NFCReader::NFCReader() : context(nullptr), device(nullptr) {}

NFCReader::~NFCReader() {
    if (device) nfc_close(device);
    if (context) nfc_exit(context);
}

bool NFCReader::connect() {
    nfc_init(&context);
    if (context == nullptr) return false;

    // This opens the device defined in /etc/nfc/libnfc.conf
    device = nfc_open(context, nullptr);
    if (device == nullptr) return false;

    // Set the ReadPi to "Initiator" mode (looking for cards)
    return (nfc_initiator_init(device) >= 0);
}

std::string NFCReader::scanCard() {
    nfc_target nt;
    const nfc_modulation nm = { .nmt = NMT_ISO14443A, .nbr = NBR_106 };

    // Poll for a card (non-blocking for 100ms)
    if (nfc_initiator_poll_target(device, &nm, 1, 1, 2, &nt) > 0) {
        std::stringstream ss;
        for (size_t i = 0; i < nt.nti.nai.szUidLen; i++) {
            ss << std::hex << std::setw(2) << std::setfill('0') 
               << (int)nt.nti.nai.abtUid[i];
        }
        return ss.str();
    }
    return ""; // Return empty string if no card found
}