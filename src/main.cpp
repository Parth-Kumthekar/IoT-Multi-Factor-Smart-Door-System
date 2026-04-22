#include "DoorAlarmSystem.h"
#include "CameraThread.h"
#include "RecognitionThread.h"
#include "ThreadSafeQueue.h"
#include "LoggerCam.h"
#include "AsyncLogger.h"
#include <iostream>
#include <csignal>
#include <condition_variable>
#include <mutex>
#include <opencv2/opencv.hpp>

// --- Global Shutdown Synchronization ---
std::condition_variable shutdown_cv;
std::mutex shutdown_mtx;
AsyncLoggerCam gLogger("database/access_log.csv");
bool should_exit = false;

/**
 * @brief Global Signal Handler for Unix signals (e.g., SIGINT).
 */
void signalHandler(int signum) {
    std::cout << "\nInterrupt signal (" << signum << ") received. Triggering Shutdown...\n";
    {
        std::lock_guard<std::mutex> lock(shutdown_mtx);
        should_exit = true;
    }
    shutdown_cv.notify_one();
}

/**
 * @brief Main Entry Point for the Multi-Factor Security System.
 * @details Spawns NFC, Alarm, Camera, and Face Recognition subsystems in parallel.
 */
int main() {
    // 1. Register SIGINT (Ctrl+C) handler for graceful exit
    signal(SIGINT, signalHandler);

    try {
        // 2. Setup Communication Bridge for Vision System
        // Passes captured frames from CameraThread to RecognitionThread
        ThreadSafeQueue<cv::Mat> frameQueue;

        // 3. Instantiate Subsystems (RAII)
        DoorAlarmSystem nfcSystem;                  // NFC, Button, and Alarm Logic
        CameraThread camera(frameQueue);            // Hardware Camera capture
        RecognitionThread recognition(frameQueue);  // AI/Face Recognition logic

        std::cout << "====================================================\n";
        std::cout << "--- INTEGRATED MULTI-FACTOR SMART DOOR SYSTEM ---\n";
        std::cout << "---       NFC + FACE-ID + MANUAL OVERRIDE      ---\n";
        std::cout << "====================================================\n";

        // 4. Start Parallel Background Services
        // These calls are non-blocking; they spawn internal worker threads.
        nfcSystem.start();   
        camera.start();      
        recognition.start(); 

        std::cout << "System is ACTIVE. Monitoring all entry points...\n";
        std::cout << "Press Ctrl+C to stop the system.\n";

        // 5. Block Main Thread until signalHandler sets should_exit
        // This keeps the program alive while workers handle hardware in parallel.
        std::unique_lock<std::mutex> lock(shutdown_mtx);
        shutdown_cv.wait(lock, []{ return should_exit; });

        // 6. Synchronized Graceful Teardown
        // Stop components in reverse order of dependency
        std::cout << "\nInitiating graceful shutdown sequence...\n";
        
        recognition.stop();
        camera.stop();
        nfcSystem.stop();

        std::cout << "All subsystems halted. System shut down cleanly.\n";

    } catch (const std::exception& e) {
        std::cerr << "FATAL SYSTEM ERROR: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
