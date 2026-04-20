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
    
    lccv::PiCamera cam;
    cam.options->camera      = deviceIndex_;   
    cam.options->video_width  = width_;
    cam.options->video_height = height_;
    cam.options->framerate    = 30;
    cam.options->verbose      = false;         

    
    cam.startVideo();
    std::cout << "LCCV started: "
              << width_ << "x" << height_ << " @ 30fps\n";

    cv::Mat frame;
    int     emptyCount = 0;

    while (running_.load()) {
        
        if (!cam.getVideoFrame(frame, 1000)) {
            ++emptyCount;
            if (emptyCount > 10) {
                std::cerr << "No frames for 10s — "
                             "check camera cable and config.txt\n";
                emptyCount = 0;
            }
            continue;
        }
        emptyCount = 0;

        if (frame.empty()) continue;

        
        queue_.pushBounded(frame.clone(), 4);
    }

    cam.stopVideo();
    
}