
#include "FaceRecognizer.h"
#include <numeric>
#include <cmath>
#include <iostream>

//FaceRecognizer 
FaceRecognizer::FaceRecognizer() = default;

bool FaceRecognizer::loadModels(const std::string& cascadePath,
                                const std::string& onnxPath) {
    if (!cascade_.load(cascadePath)) {
        std::cerr << "[FaceRecognizer] Cannot load cascade: " << cascadePath << "\n";
        return false;
    }
    net_ = cv::dnn::readNetFromONNX(onnxPath);
    if (net_.empty()) {
        std::cerr << "[FaceRecognizer] Cannot load ONNX model: " << onnxPath << "\n";
        return false;
    }
    net_.setPreferableBackend(cv::dnn::DNN_BACKEND_DEFAULT);
    net_.setPreferableTarget(cv::dnn::DNN_TARGET_CPU);
    std::cout << "[FaceRecognizer] Models loaded OK\n";
    return true;
}

bool FaceRecognizer::loadDatabase(const std::string& dbPath) {
    cv::FileStorage fs(dbPath, cv::FileStorage::READ); //Calling the database from the storage
    if (!fs.isOpened()) {
        std::cerr << "[FaceRecognizer] Cannot open database: " << dbPath << "\n";
        return false;
    }
    database_.clear();
    cv::FileNode people = fs["people"];
    for (auto it = people.begin(); it != people.end(); ++it) {
        FaceEntry entry;
        entry.name = (std::string)(*it)["name"];
        cv::Mat embMat;
        (*it)["embedding"] >> embMat;
        entry.embedding.assign(embMat.begin<float>(), embMat.end<float>());
        database_.push_back(std::move(entry));
    }
    std::cout << "[FaceRecognizer] Loaded " << database_.size()
              << " entries from database\n";
    return !database_.empty();
}

std::vector<RecognitionResult> FaceRecognizer::process(const cv::Mat& frame) {
    std::vector<RecognitionResult> results;
    cv::Mat gray;
    cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);
    cv::equalizeHist(gray, gray);

    

    if (frameCount_ % kDetectEvery == 0) {
        cascade_.detectMultiScale(gray, lastFaces_, 1.1, 5,
                                  0, cv::Size(80, 80));
    }
    ++frameCount_;

    for (const auto& rect : lastFaces_) {
        cv::Mat faceROI = frame(rect).clone();

        RecognitionResult res;
        res.faceRect   = rect;
        res.name       = "Unknown";
        res.confidence = 0.f;
        res.unlocked   = false;

        if (!database_.empty()) {
            std::vector<float> emb = getEmbedding(faceROI);
            float bestScore = -1.f;
            for (const auto& entry : database_) {
                float score = cosineSimilarity(emb, entry.embedding);
                if (score > bestScore) {
                    bestScore = score;
                    if (score >= kMatchThreshold) {
                        res.name       = entry.name;
                        res.confidence = score;
                    }
                }
            }
        }
        results.push_back(std::move(res));
    }
    return results;
}

std::vector<float> FaceRecognizer::getEmbedding(const cv::Mat& face) {
    cv::Mat resized;
    cv::resize(face, resized, cv::Size(112, 112));
    cv::Mat blob = cv::dnn::blobFromImage(resized, 1.0 / 128.0,
                                          cv::Size(112, 112),
                                          cv::Scalar(127.5, 127.5, 127.5),
                                          true, false);
    net_.setInput(blob);
    cv::Mat output = net_.forward();

    std::vector<float> emb(output.begin<float>(), output.end<float>());
    return emb;
}

float FaceRecognizer::cosineSimilarity(const std::vector<float>& a,
                                       const std::vector<float>& b) {
    if (a.size() != b.size() || a.empty()) return 0.f;
    float dot  = 0.f, na = 0.f, nb = 0.f;
    for (size_t i = 0; i < a.size(); ++i) {
        dot += a[i] * b[i];
        na  += a[i] * a[i];
        nb  += b[i] * b[i];
    }
    float denom = std::sqrt(na) * std::sqrt(nb);
    return (denom < 1e-9f) ? 0.f : dot / denom;
}

// FaceSystem 

FaceSystem::FaceSystem(ThreadSafeQueue<cv::Mat>& queue)
    : queue_(queue) {
    recognizer_.loadModels("models/haarcascade_frontalface_default.xml",
                           "models/face_recognition.onnx");
    recognizer_.loadDatabase("database/embeddings.yml");
}

void FaceSystem::run() {
    while (true) {
        cv::Mat frame;
        try { frame = queue_.pop(); }
        catch (...) { break; }

        auto results = recognizer_.process(frame);

        for (auto& res : results) {
            // Draw bounding box
            cv::Scalar colour = (res.name != "Unknown")
                                ? cv::Scalar(0, 255, 0)
                                : cv::Scalar(0, 0, 255);
            cv::rectangle(frame, res.faceRect, colour, 2);

            std::string label = res.name;
            if (res.confidence > 0.f)
                label += " (" + std::to_string((int)(res.confidence * 100)) + "%)";
            cv::putText(frame, label,
                        cv::Point(res.faceRect.x, res.faceRect.y - 8),
                        cv::FONT_HERSHEY_SIMPLEX, 0.6, colour, 2);

            // Attempt door unlock for matched persons
            if (res.name != "Unknown") {
                res.unlocked = doorLock_.unlock(res.name);
                if (res.unlocked) {
                    cv::putText(frame, "UNLOCKED",
                                cv::Point(20, 50),
                                cv::FONT_HERSHEY_SIMPLEX, 1.2,
                                cv::Scalar(0, 255, 0), 3);
                }
            }
        }

        cv::imshow("FaceID Door Lock", frame);
        if (cv::waitKey(1) == 27) { // ESC to quit
            queue_.shutdown();
            break;
        }
    }
    cv::destroyAllWindows();
}