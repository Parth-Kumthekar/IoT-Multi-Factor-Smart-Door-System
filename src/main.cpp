#include "DoorController.hpp"
#include "OutputHandler.hpp"
#include <iostream>

int main() {
    // Pin setup for Pi 5
    DoorController door(11); // Reed
    OutputHandler  hw(31, 13, 15, 37); // Relay, Green, Red, Buzzer

    if (!door.initialize() || !hw.init()) {
        return 1;
    }

    hw.lock();
    std::string authorizedUID = "046732ca"; // Replace with your card's actual UID

    while (true) {
        door.checkDoorStatus();
        
        std::string uid = door.scanNFC();
        if (!uid.empty()) {
            std::cout << "Detected: " << uid << std::endl;
            if (uid == authorizedUID) {
                hw.setAccessGranted();
                std::this_thread::sleep_for(std::chrono::seconds(5));
                hw.lock();
            } else {
                hw.setAccessDenied();
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    return 0;
}