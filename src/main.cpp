#include "DoorController.hpp"
#include "OutputHandler.hpp"
#include <iostream>
#include <thread>
#include <chrono>
#include <csignal>

// Global flag to allow clean exit with Ctrl+C
bool keepRunning = true;
void signalHandler(int signum) {
    keepRunning = false;
}

int main() {
    // 1. Register signal handler for Ctrl+C
    std::signal(SIGINT, signalHandler);

    // 2. Define Pins (Using BCM numbering for Pi 5)
    // Adjust these to match your actual wiring!
    unsigned int REED_PIN   = 11; // GPIO 11
    unsigned int RELAY_PIN  = 31; // GPIO 6 (Pin 31)
    unsigned int GREEN_LED  = 13; // GPIO 27 (Example)
    unsigned int RED_LED    = 15; // GPIO 22 (Example)
    unsigned int BUZZER_PIN = 37; // GPIO 26 (Example)

    // 3. Instantiate Handlers
    DoorController door(REED_PIN);
    OutputHandler  hw(RELAY_PIN, GREEN_LED, RED_LED, BUZZER_PIN);

    // 4. Initialize Hardware
    std::cout << "--- Starting Smart Door System ---" << std::endl;
    
    if (!hw.init()) {
        std::cerr << "FAILED to initialize Output Hardware. Check sudo/permissions." << std::endl;
        return 1;
    }

    if (!door.initialize()) {
        std::cerr << "FAILED to initialize Door/NFC Controller." << std::endl;
        return 1;
    }

    // Ensure door starts in LOCKED state
    hw.lock();

    // 5. Main Execution Loop
    std::cout << "System Active. Press Ctrl+C to exit." << std::endl;

    while (keepRunning) {
        // Check if the door is physically open or closed
        door.checkDoorStatus();

        // Check for NFC Tags (This will print UID and validate)
        // In a full implementation, you'd pass 'hw' to 'door' 
        // so it can trigger the relay.
        door.processNFC();

        // Small heartbeat delay to prevent 100% CPU usage
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    // 6. Cleanup on Exit
    std::cout << "\nShutting down system..." << std::endl;
    hw.lock(); // Ensure door is locked before program closes
    
    return 0;
}