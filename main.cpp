#include <thread>
#include <iostream>
#include "ThreadSafeQueue.h"
#include "FaceRecognizer.h"

extern void cameraThread(ThreadSafeQueue<cv::Mat>&);

int main() {
    ThreadSafeQueue<cv::Mat> queue;

    auto callback = [](const std::string& name) {
        std::cout << "Detected: " << name << std::endl;
        };

    FaceRecognizer recognizer(queue, callback);

    std::thread camThread(cameraThread, std::ref(queue));
    std::thread workerThread(&FaceRecognizer::start, &recognizer);

    camThread.join();
    workerThread.join();

    return 0;

}