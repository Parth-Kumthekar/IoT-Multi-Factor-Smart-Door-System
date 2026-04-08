#include "AccessController.hpp"
#include <iostream>

void AccessController::init() {
    nfc.init();
    output.init();
}

void AccessController::run() {
    while (true) {
        std::string uid = nfc.readUID();

        if (uid.empty()) continue;

        std::cout << "UID: " << uid << std::endl;

        if (uid == validUID) {
            std::cout << "Access Granted\n";
            output.accessGranted();
        } else {
            std::cout << "Access Denied\n";
            output.accessDenied();
        }
    }
}