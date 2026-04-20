#include "DoorAlarmSystem.h"
#include <iostream>
#include <csignal>
#include <condition_variable>
#include <mutex>

/// Condition variable used to block the main thread until a shutdown signal is received.
std::condition_variable shutdown_cv;

/// Mutex to protect the shutdown flag.
std::mutex shutdown_mtx;

/// Global flag indicating if the application should terminate.
bool should_exit = false;

/**
 * @brief Global Signal Handler for Unix signals (e.g., SIGINT).
 * @details When the user presses Ctrl+C, this function sets the should_exit flag 
 * and notifies the main thread to begin the teardown process.
 * @param signum The signal identifier (e.g., 2 for SIGINT).
 */
void signalHandler(int signum) {
    std::cout << "\nInterrupt signal (" << signum << ") received. Shutting down...\n";
    {
        std::lock_guard<std::mutex> lock(shutdown_mtx);
        should_exit = true;
    }
    shutdown_cv.notify_one();
}

/**
 * @brief Application Entry Point.
 * @details 
 * 1. Registers the signal handler for graceful termination.
 * 2. Instantiates the DoorAlarmSystem (RAII).
 * 3. Starts the system threads and hardware monitoring.
 * 4. Enters a blocked state, waiting for a shutdown signal.
 * 5. Calls system.stop() to ensure all hardware and threads are released cleanly.
 * * @return int 0 on successful execution, 1 on fatal exceptions.
 */
int main() {
    // Register SIGINT (Ctrl+C) handler
    signal(SIGINT, signalHandler);

    try {
        DoorAlarmSystem system;
        
        std::cout << "--- Raspberry Pi 5 Smart Door System Starting ---\n";
        system.start();
        
        std::cout << "System is ACTIVE. Press Ctrl+C to exit.\n";

        // Main thread waits here effectively forever until signalHandler is called
        std::unique_lock<std::mutex> lock(shutdown_mtx);
        shutdown_cv.wait(lock, []{ return should_exit; });

        // Begin graceful teardown
        system.stop();
        std::cout << "System shut down cleanly.\n";

    } catch (const std::exception& e) {
        std::cerr << "FATAL ERROR: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
