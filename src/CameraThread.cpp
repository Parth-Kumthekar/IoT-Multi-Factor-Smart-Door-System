#include "CameraThread.h"
#include <iostream>
#include <stdexcept>
#include <chrono>
#include <thread>
#include <lccv.hpp>

CameraThread::CameraThread(ThreadSafeQueue<cv::Mat>& queue,
                           int deviceIndex, int width, int height)
    : queue_(queue)
    , deviceIndex_(deviceIndex)
    , width_(width)
    , height_(height) {}

CameraThread::~CameraThread() { stop(); }

void CameraThread::start() {
    running_.store(true);
    thread_ = std::thread(&CameraThread::loop, this);
}

void CameraThread::stop() {
    running_.store(false);
    queue_.shutdown();
    if (thread_.joinable())
        thread_.join();
}

void CameraThread::loop() {
    
     try {
        std::string pipeline =
            "libcamerasrc ! video/x-raw,width=640,height=480,framerate=30/1 "
            "! videoconvert ! appsink";

        cv::VideoCapture cap(pipeline, cv::CAP_GSTREAMER);

        if (!cap.isOpened()) {
            std::cerr << "[CAMERA ERROR] Failed to open via GStreamer\n";
            return;
        }

        std::cout << "[CAMERA] Using libcamera (GStreamer pipeline)\n";

        cv::Mat frame;

        while (running_.load()) {

            if (!cap.read(frame)) {
                std::cerr << "[CAMERA ERROR] Frame read failed\n";
                continue;
            }

            if (frame.empty()) {
                std::cerr << "[CAMERA WARNING] Empty frame\n";
                continue;
            }

            queue_.pushBounded(frame.clone(), 4);
        }

        cap.release();
        std::cout << "[CAMERA] Stopped\n";
    }
    catch (const std::exception& e) {
        std::cerr << "[CAMERA EXCEPTION] " << e.what() << std::endl;
    }
}