#include "DoorController.hpp"
#include "OutputHandler.hpp"
#include <thread>
#include <chrono>

int main() {
    // Pin Definitions for Pi 5
    const int REED_PIN = 17;
    const int RELAY_PIN = 6;
    const int GREEN_LED = 26;
    const int RED_LED = 19;
    const int BUZZER_PIN = 13;

    DoorController reader(REED_PIN);
    OutputHandler hardware(RELAY_PIN, GREEN_LED, RED_LED, BUZZER_PIN);

    if (!reader.initialize() || !hardware.init()) {
        std::cerr << "Hardware Initialization Failed!" << std::endl;
        return 1;
    }

    hardware.lock(); // Start in locked state (Red LED on)

    while (true) {
        reader.checkDoorStatus(); // Monitor Reed Switch
        reader.processNFC(hardware); // Pass output object to NFC logic
        
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    return 0;
}