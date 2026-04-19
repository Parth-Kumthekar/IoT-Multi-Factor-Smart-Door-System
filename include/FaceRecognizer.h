#pragma once
#include <opencv2/opencv.hpp>
#include <opencv2/dnn.hpp>
#include <vector>
#include <string>
#include "ThreadSafeQueue.h"
#include "AccessEvent.h"

//  run the heavy DNN embedding 
static constexpr int   kDetectEvery    = 3;
static constexpr float kMatchThreshold = 0.45f;

struct FaceEntry {
    std::string        name;
    std::vector<float> embedding;
};

class FaceRecognizer {
public:
    FaceRecognizer();
    ~FaceRecognizer() = default;

    FaceRecognizer(const FaceRecognizer&)            = delete;
    FaceRecognizer& operator=(const FaceRecognizer&) = delete;

    [[nodiscard]] bool loadModels(const std::string& cascadePath,
                                  const std::string& onnxPath);
    [[nodiscard]] bool loadDatabase(const std::string& dbPath);

    // Called on recognition thread 
    std::vector<AccessEvent> process(const cv::Mat& frame);

private:
    std::vector<float> getEmbedding(const cv::Mat& face);
    float cosineSimilarity(const std::vector<float>& a,
                           const std::vector<float>& b) const;

    cv::CascadeClassifier  cascade_;
    cv::dnn::Net           net_;
    std::vector<FaceEntry> database_;
    int                    frameCount_{0};
    std::vector<cv::Rect>  lastFaces_;
};