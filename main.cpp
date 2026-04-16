
#include <opencv2/opencv.hpp>
#include <thread>
#include "ThreadSafeQueue.h"
#include "FaceRecognizer.h"

int main() {

    GUIServer guiServer(8080); //GUI Server for the facial recognition
    guiServer.start();

    ThreadSafeQueue<cv::Mat> frameQueue;
    FaceSystem system(frameQueue);

    std::thread cam([&]() {
        cv::VideoCapture cap(0, cv::CAP_V4L2);
        if (!cap.isOpened()) {
            throw std::runtime_error("Camera not opened");
        }
        cap.set(cv::CAP_PROP_FRAME_WIDTH,  640);
        cap.set(cv::CAP_PROP_FRAME_HEIGHT, 480);

        while (true) {
            cv::Mat frame;
            cap >> frame;
            if (!frame.empty())
                frameQueue.push(frame);
        }
    });

    std::thread worker(&FaceSystem::run, &system);

    cam.join();
    worker.join();
    return 0;
}