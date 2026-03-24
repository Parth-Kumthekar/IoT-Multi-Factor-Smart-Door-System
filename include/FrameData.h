#pragma once
#include <opencv2/opencv.hpp>
#include <vector>
#include <string>

struct FrameData {
    cv::Mat frame;
    std::vector<cv::Rect> faces;
};