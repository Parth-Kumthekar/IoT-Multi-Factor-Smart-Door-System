#include "DoorAlarmSystem.h"
#include <iostream>
#include <csignal>
#include <atomic>

// Atomic flag to handle Ctrl+C gracefully
std::atomic<bool> keepRunning(true);

void signalHandler(int signum) {
    std::cout << "\nInterrupt signal (" << signum << ") received. Shutting down...\n";
    keepRunning = false;
}

int main() {
    // Register signal handler for Ctrl+C
    signal(SIGINT, signalHandler);

    try {
        DoorAlarmSystem system;
        
        std::cout << "--- Raspberry Pi 5 Smart Door System Starting ---\n";
        system.start();
        
        std::cout << "System is ACTIVE.\n";
        std::cout << "Monitoring Door Sensor and NFC Reader...\n";
        std::cout << "Press Ctrl+C to exit.\n";

        // Main thread stays alive while background threads do the work
        while (keepRunning) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }

        system.stop();
        std::cout << "System shut down cleanly.\n";

    } catch (const std::exception& e) {
        std::cerr << "FATAL ERROR: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}