#pragma once
#include <thread>
#include <atomic>
#include <opencv2/opencv.hpp>
#include "ThreadSafeQueue.h"
#include "FaceRecognizer.h"

class RecognitionThread {
public:
    explicit RecognitionThread(ThreadSafeQueue<cv::Mat>& queue);
    ~RecognitionThread();  

    RecognitionThread(const RecognitionThread&)            = delete;
    RecognitionThread& operator=(const RecognitionThread&) = delete;

    void start();
    void stop();

private:
    void loop();
    void drawOverlay(cv::Mat& frame, const std::vector<AccessEvent>& events);

    ThreadSafeQueue<cv::Mat>& queue_;
    FaceRecognizer            recognizer_;
    std::atomic<bool>         running_{false};
    std::thread               thread_;
};