#include "DoorController.hpp"
#include <thread>
#include <chrono>
#include <csignal>
#include <iostream>

bool running = true;
void handleExit(int s) { running = false; }

int main() {
    std::signal(SIGINT, handleExit);

    DoorController door(17); // Use BCM Pin 17

    if (!door.initialize()) {
        std::cerr << "System startup failed. Check wiring/I2C." << std::endl;
        return 1;
    }

    std::cout << "System running. Monitoring Door and NFC..." << std::endl;

    while (running) {
        door.checkDoorStatus();
        door.processNFC();
        
        // Balance between responsiveness and CPU usage
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    return 0;
}