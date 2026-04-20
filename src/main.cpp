#include "DoorAlarmSystem.h"
#include <iostream>
#include <csignal>
#include <condition_variable>
#include <mutex>

// Global primitives for synchronization between the Signal Handler and Main thread
std::condition_variable shutdown_cv;
std::mutex shutdown_mtx;
bool should_exit = false;

/**
 * @brief Handles OS signals like SIGINT (Ctrl+C).
 * * Instead of calling exit() immediately, we signal the main thread to 
 * perform a graceful cleanup of hardware and network resources.
 */
void signalHandler(int signum) {
    std::cout << "\n[SIGNAL] Interrupt (" << signum << ") detected. Initiating shutdown...\n";
    {
        std::lock_guard<std::mutex> lock(shutdown_mtx);
        should_exit = true;
    }
    shutdown_cv.notify_one();
}

/**
 * @brief Entry point for the Smart Door Security System.
 * * This function manages the high-level lifecycle of the application:
 * 1. Signal registration
 * 2. System orchestration
 * 3. Graceful resource release
 */
int main() {
    // Register SIGINT handler to allow for a clean exit via Ctrl+C
    signal(SIGINT, signalHandler);

    try {
        // Instantiate the main orchestrator (RAII)
        DoorAlarmSystem system;
        
        std::cout << "===============================================\n";
        std::cout << "   Raspberry Pi 5 Smart Door System Starting   \n";
        std::cout << "===============================================\n";
        
        // Non-blocking start: launches all hardware/logic threads
        system.start();
        
        std::cout << "System is now MONITORING. Press Ctrl+C to terminate.\n";

        // Main thread suspension: Sleeps efficiently until should_exit is true.
        // This fulfills the "High Reliability" requirement by using 0% CPU while idle.
        std::unique_lock<std::mutex> lock(shutdown_mtx);
        shutdown_cv.wait(lock, []{ return should_exit; });

        // Cleanup: Join threads, release GPIO, close network sockets
        system.stop();
        std::cout << "Shutdown sequence complete. Hardware released.\n";

    } catch (const std::exception& e) {
        // Top-level exception handling to prevent silent system crashes
        std::cerr << "FATAL SYSTEM ERROR: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}