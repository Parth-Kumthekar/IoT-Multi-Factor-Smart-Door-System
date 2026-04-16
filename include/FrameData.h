
#pragma once
#include <opencv2/opencv.hpp>
#include <chrono>

struct FrameData {
    cv::Mat frame;
    std::chrono::steady_clock::time_point timestamp;

    FrameData() = default;
    explicit FrameData(cv::Mat f)
        : frame(std::move(f)),
          timestamp(std::chrono::steady_clock::now()) {}
};