#include "NFCReader.hpp"
#include "OutputController.hpp"
#include "AccessController.hpp"

#include <iostream>
#include <thread>

int main() {
    NFCReader nfc;
    OutputController output;
    AccessController access;

    if (!nfc.init()) {
        std::cerr << "NFC init failed\n";
        return -1;
    }

    output.init();

    while (true) {
        std::string uid = nfc.readUID();

        if (!uid.empty()) {
            std::cout << "UID: " << uid << std::endl;

            if (access.isAuthorized(uid)) {
                std::cout << "ACCESS GRANTED\n";
                output.accessGranted();
            } else {
                std::cout << "ACCESS DENIED\n";
                output.accessDenied();
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

    return 0;
}