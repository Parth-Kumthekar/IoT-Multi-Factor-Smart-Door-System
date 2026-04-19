#include "FaceRecognizer.h"
#include "EventBus.h"
#include "OverrideManager.h"
#include <cmath>
#include <iostream>

FaceRecognizer::FaceRecognizer() = default;

bool FaceRecognizer::loadModels(const std::string& cascadePath,
                                const std::string& onnxPath) {
    if (!cascade_.load(cascadePath)) {
        std::cerr << "[Face] Cannot load cascade: " << cascadePath << '\n';
        return false;
    }
    net_ = cv::dnn::readNetFromONNX(onnxPath);
    if (net_.empty()) {
        std::cerr << "[Face] Cannot load ONNX: " << onnxPath << '\n';
        return false;
    }
    net_.setPreferableBackend(cv::dnn::DNN_BACKEND_DEFAULT);
    net_.setPreferableTarget(cv::dnn::DNN_TARGET_CPU);
    std::cout << "[Face] Models loaded\n";
    return true;
}

bool FaceRecognizer::loadDatabase(const std::string& dbPath) {
    cv::FileStorage fs(dbPath, cv::FileStorage::READ); //File reader
    if (!fs.isOpened()) {
        std::cerr << "[Face] Cannot open DB: " << dbPath << '\n';
        return false;
    }
    database_.clear();
    for (auto it = fs["people"].begin(); it != fs["people"].end(); ++it) {
        FaceEntry e;
        e.name = (std::string)(*it)["name"];
        cv::Mat m; (*it)["embedding"] >> m;
        e.embedding.assign(m.begin<float>(), m.end<float>());
        database_.push_back(std::move(e));
    }
    std::cout << "[Face] Loaded " << database_.size() << " identities\n";
    return !database_.empty();
}

std::vector<AccessEvent> FaceRecognizer::process(const cv::Mat& frame) {
    std::vector<AccessEvent> events;
    cv::Mat gray;
    cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);
    cv::equalizeHist(gray, gray);

    if (frameCount_++ % kDetectEvery == 0)
        cascade_.detectMultiScale(gray, lastFaces_, 1.1, 5, 0, {80,80});

    for (const auto& rect : lastFaces_) {
        // Override check — if active, skip recognition and grant access
        if (OverrideManager::instance().isActive()) {
            auto ev = AccessEvent::override_("Override", "Bypass active");
            ev.faceRect = rect;
            events.push_back(std::move(ev));
            continue;
        }

        cv::Mat roi = frame(rect).clone();
        auto emb    = getEmbedding(roi);

        std::string bestName  = "Unknown";
        float       bestScore = 0.f;
        for (const auto& entry : database_) {
            float s = cosineSimilarity(emb, entry.embedding);
            if (s > bestScore) {
                bestScore = s;
                if (s >= kMatchThreshold) bestName = entry.name;
            }
        }

        AccessEvent ev;
        ev.timestamp  = std::chrono::system_clock::now();
        ev.method     = AuthMethod::FACE_ID;
        ev.identity   = bestName;
        ev.confidence = bestScore;
        ev.faceRect   = rect;
        ev.result     = (bestName != "Unknown")
                        ? AuthResult::GRANTED : AuthResult::DENIED;
        events.push_back(std::move(ev));
    }
    return events;
}

std::vector<float> FaceRecognizer::getEmbedding(const cv::Mat& face) {
    cv::Mat resized, blob;
    cv::resize(face, resized, {112, 112});
    blob = cv::dnn::blobFromImage(resized, 1.0/128.0, {112,112},
                                  {127.5,127.5,127.5}, true, false);
    net_.setInput(blob);
    cv::Mat out = net_.forward();
    return {out.begin<float>(), out.end<float>()};
}

float FaceRecognizer::cosineSimilarity(const std::vector<float>& a,
                                       const std::vector<float>& b) const {
    if (a.size() != b.size() || a.empty()) return 0.f;
    float dot=0, na=0, nb=0;
    for (size_t i=0; i<a.size(); ++i) {
        dot += a[i]*b[i]; na += a[i]*a[i]; nb += b[i]*b[i];
    }
    float d = std::sqrt(na)*std::sqrt(nb);
    return d < 1e-9f ? 0.f : dot/d;
}