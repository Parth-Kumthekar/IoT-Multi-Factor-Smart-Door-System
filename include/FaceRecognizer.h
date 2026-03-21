#pragma once
#include <opencv2/opencv.hpp>
#include <opencv2/dnn.hpp>
#include <map>
#include "ThreadSafeQueue.h"
#include "Callback.h"

class FaceRecognizer {
private:
    ThreadSafeQueue<cv::Mat>& queue_;
    FaceCallback callback_;

    cv::CascadeClassifier faceCascade_;
    cv::dnn::Net recognitionNet_;

    std::map<std::string, cv::Mat> database_;
    bool running_;

    cv::Mat getEmbedding(cv::Mat& face);
    std::string recognizeFace(cv::Mat& embedding);
    double cosineSimilarity(const cv::Mat& a, const cv::Mat& b);

public:
    FaceRecognizer(ThreadSafeQueue<cv::Mat>& queue, FaceCallback cb);
    void start();
};