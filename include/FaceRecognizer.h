#pragma once
#include <opencv2/opencv.hpp>
#include <opencv2/dnn.hpp>
#include <string>
#include <vector>
#include <unordered_map>
#include "ThreadSafeQueue.h"
#include "DoorLock.h"
#include "Callback.h"

// How often to run the heavy DNN embedding (every N frames)
static constexpr int kDetectEvery     = 3;
// Cosine similarity threshold — raise to reduce false accepts
static constexpr float kMatchThreshold = 0.45f;

struct FaceEntry {
    std::string        name;
    std::vector<float> embedding;
};

class FaceRecognizer {
public:
    FaceRecognizer();

    // Load Haar cascade and ONNX embedding model
    bool loadModels(const std::string& cascadePath,
                    const std::string& onnxPath);

    // Load embeddings database built by build_database
    bool loadDatabase(const std::string& dbPath);

    // Detect faces in frame, return all results
    std::vector<RecognitionResult> process(const cv::Mat& frame);

private:
    std::vector<float> getEmbedding(const cv::Mat& face);
    float cosineSimilarity(const std::vector<float>& a,
                           const std::vector<float>& b);

    cv::CascadeClassifier      cascade_;
    cv::dnn::Net               net_;
    std::vector<FaceEntry>     database_;
    int                        frameCount_ = 0;
    std::vector<cv::Rect>      lastFaces_;
};


class FaceSystem {
public:
    explicit FaceSystem(ThreadSafeQueue<cv::Mat>& queue);
    void run();

private:
    ThreadSafeQueue<cv::Mat>& queue_;
    FaceRecognizer            recognizer_;
    DoorLock                  doorLock_;
};