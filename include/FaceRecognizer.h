#pragma once
#include <opencv2/opencv.hpp>
#include "ThreadSafeQueue.h"
#include "FrameData.h"

class FaceSystem {
private:
    ThreadSafeQueue<cv::Mat>& input_;
    cv::CascadeClassifier cascade_;

public:
    FaceSystem(ThreadSafeQueue<cv::Mat>& queue);
    void run();
};