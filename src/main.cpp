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

// Shared shutdown variables
std::condition_variable shutdown_cv;
std::mutex shutdown_mtx;
bool should_exit = false;
AsyncLoggerCam gLogger("database/access_log.csv");
/**
 * @brief Global Signal Handler for Unix signals (e.g., SIGINT).
 */
void signalHandler(int signum) {
    std::cout << "\nInterrupt signal (" << signum << ") received. Shutting down...\n";
    {
        std::lock_guard<std::mutex> lock(shutdown_mtx);
        should_exit = true;
    }
    shutdown_cv.notify_one();
}

int main() {
    // 1. Register SIGINT (Ctrl+C) handler
    signal(SIGINT, signalHandler);

    try {
        // 2. Setup Communication Bridge (The Queue)
        // Passes frames from Camera -> Recognition AI
        ThreadSafeQueue<cv::Mat> frameQueue;

        // 3. Instantiate All Subsystems
        DoorAlarmSystem nfcSystem;            // NFC & Alarm hardware logic
        CameraThread    camera(frameQueue);   // Hardware Camera capture
        RecognitionThread recognition(frameQueue); // Face Recognition logic

        std::cout << "--- Integrated Multi-Factor Smart Door System Starting ---\n";
        std::cout << "FaceID + NFC Monitoring Enabled.\n";

        // 4. Start Threads
        nfcSystem.start();   
        camera.start();      
        recognition.start(); 

        std::cout << "System is ACTIVE. Press Ctrl+C to exit.\n";

        // 5. Block Main Thread until signalHandler is called
        std::unique_lock<std::mutex> lock(shutdown_mtx);
        shutdown_cv.wait(lock, []{ return should_exit; });

        // 6. Graceful Teardown
        std::cout << "Shutting down subsystems...\n";
        recognition.stop();
        camera.stop();
        nfcSystem.stop();

        std::cout << "System shut down cleanly.\n";

    } catch (const std::exception& e) {
        std::cerr << "FATAL ERROR: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
