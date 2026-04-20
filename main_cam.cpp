#include <opencv2/opencv.hpp>
#include <iostream>
#include <thread>
#include <chrono>

#include "ThreadSafeQueue.h"
#include "AccessEvent.h"
#include "EventBus.h"
#include "Logger.h"
#include "DoorLock.h"
#include "OverrideManager.h"
#include "CameraThread.h"
#include "RecognitionThread.h"
#include "SignalHandler.h"
#include "GUIServer.h"


// Global logger 
AsyncLogger gLogger("database/access_log.csv");

int main() {

    installSignalHandlers();

    std::cout << "FaceID + NFC Door Lock starting\n";

    
    DoorController door;       
    GUIServer      gui(8080);   

    // Camera pipeline
    ThreadSafeQueue<cv::Mat> frameQueue;
    CameraThread       camera(frameQueue);
    RecognitionThread  recognition(frameQueue);

    gui.start();
    camera.start();
    recognition.start();

    std::cout << "All subsystems running. "
                 "GUI at http://<pi-ip>:8080 | ESC in preview to quit\n";

    // Main thread(SIGINT/SIGTERM or ESC)
    while (!gShutdown.load()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }

    // Clean shutdown 
    std::cout << "Shutting down...\n";
    recognition.stop();   
    camera.stop();                  
    gui.stop();           
    

    std::cout << " exit\n";
    return 0;
}