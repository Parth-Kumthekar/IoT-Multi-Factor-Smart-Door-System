#pragma once
#include <thread>
#include <atomic>
#include <opencv2/opencv.hpp>
#include "ThreadSafeQueue.h"

class CameraThread {
public:
    explicit CameraThread(ThreadSafeQueue<cv::Mat>& queue,
                          int deviceIndex = 0,
                          int width       = 640,
                          int height      = 480);
    ~CameraThread();

    CameraThread(const CameraThread&)            = delete;
    CameraThread& operator=(const CameraThread&) = delete;

    void start();
    void stop();
    bool isRunning() const { return running_.load(); }

private:
    void loop();

    ThreadSafeQueue<cv::Mat>& queue_;
    int               deviceIndex_;
    int               width_;
    int               height_;
    std::atomic<bool> running_{false};
    std::thread       thread_;
};