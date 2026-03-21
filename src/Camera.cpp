#include <opencv2/opencv.hpp>
#include "ThreadSafeQueue.h"

void cameraThread(ThreadSafeQueue<cv::Mat>& queue) {
    cv::VideoCapture cap(0);

    if (!cap.isOpened()) {
        throw std::runtime_error("Camera not opened");
    }

    while (true) {
        cv::Mat frame;
        cap >> frame; // blocking

        if (!frame.empty()) {
            queue.push(frame);
        }
    }
}