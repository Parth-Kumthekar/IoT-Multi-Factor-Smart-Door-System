#include "DoorAlarmSystem.h"
#include <iostream>
#include <csignal>
#include <condition_variable>
#include <mutex>

std::condition_variable shutdown_cv;
std::mutex shutdown_mtx;
bool should_exit = false;

void signalHandler(int signum) {
    std::cout << "\nInterrupt signal (" << signum << ") received. Shutting down...\n";
    {
        std::lock_guard<std::mutex> lock(shutdown_mtx);
        should_exit = true;
    }
    shutdown_cv.notify_one();
}

int main() {
    signal(SIGINT, signalHandler);

    try {
        DoorAlarmSystem system;
        
        std::cout << "--- Raspberry Pi 5 Smart Door System Starting ---\n";
        system.start();
        
        std::cout << "System is ACTIVE. Press Ctrl+C to exit.\n";

        // Main thread waits here effectively forever until signalHandler is called
        std::unique_lock<std::mutex> lock(shutdown_mtx);
        shutdown_cv.wait(lock, []{ return should_exit; });

        system.stop();
        std::cout << "System shut down cleanly.\n";

    } catch (const std::exception& e) {
        std::cerr << "FATAL ERROR: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}