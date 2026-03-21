#include "FaceRecognizer.h"
#include <iostream>
#include <opencv2/imgproc.hpp>

FaceRecognizer::FaceRecognizer(ThreadSafeQueue<cv::Mat>& queue, FaceCallback cb)
    : queue_(queue), callback_(cb), running_(true)
{
    // Load Haar cascade
    if (!faceCascade_.load("models/haarcascade_frontalface_default.xml")) {
        std::cerr << "Failed to load Haar cascade!\n";
    }

    // Load ONNX model
    recognitionNet_ = cv::dnn::readNetFromONNX("models/face_recognition.onnx");

    if (recognitionNet_.empty()) {
        std::cerr << "Failed to load ONNX model!\n";
    }

    recognitionNet_.setPreferableBackend(cv::dnn::DNN_BACKEND_OPENCV);
    recognitionNet_.setPreferableTarget(cv::dnn::DNN_TARGET_CPU);

    // 
    cv::FileStorage fs("database/embeddings.yml", cv::FileStorage::READ);

    if (!fs.isOpened()) {
        std::cerr << "Failed to open embeddings.yml\n";
        return;
    }

    for (auto it = fs.root().begin(); it != fs.root().end(); ++it) {
        std::string name = (*it).name();
        cv::Mat emb;
        (*it) >> emb;
        database_[name] = emb;
    }

    std::cout << "Loaded " << database_.size() << " embeddings\n";
}

// ---------------- EMBEDDING ----------------
cv::Mat FaceRecognizer::getEmbedding(cv::Mat& face) {

    cv::resize(face, face, cv::Size(112, 112));

    if (face.channels() == 1)
        cv::cvtColor(face, face, cv::COLOR_GRAY2BGR);

    cv::cvtColor(face, face, cv::COLOR_BGR2RGB);

    face.convertTo(face, CV_32F, 1.0 / 255.0);

    cv::Mat blob = cv::dnn::blobFromImage(
        face,
        1.0,
        cv::Size(112, 112),
        cv::Scalar(0, 0, 0),
        false,
        false
    );

    recognitionNet_.setInput(blob);

    cv::Mat embedding = recognitionNet_.forward();

    cv::normalize(embedding, embedding);

    return embedding.reshape(1, 1); // flatten
}

//COSINE
double FaceRecognizer::cosineSimilarity(const cv::Mat& a, const cv::Mat& b) {

    if (a.size != b.size) {
        std::cerr << "Embedding size mismatch! Skipping...\n";
        return -1.0;
    }

    return a.dot(b) / (cv::norm(a) * cv::norm(b));
}

// ---------------- RECOGNITION ----------------
std::string FaceRecognizer::recognizeFace(cv::Mat& embedding) {

    double bestScore = 0.0;
    std::string bestName = "Unknown";

    for (auto& [name, dbEmb] : database_) {

        double score = cosineSimilarity(embedding, dbEmb);

        if (score > bestScore) {
            bestScore = score;
            bestName = name;
        }
    }

    if (bestScore > 0.6)
        return bestName;

    return "Unknown";
}

// ---------------- MAIN LOOP ----------------
void FaceRecognizer::start() {

    while (running_) {

        cv::Mat frame = queue_.pop();

        std::vector<cv::Rect> faces;
        faceCascade_.detectMultiScale(frame, faces);

        for (auto& face : faces) {

            cv::Mat roi = frame(face).clone();

            cv::Mat embedding = getEmbedding(roi);
            std::string name = recognizeFace(embedding);

            callback_(name);

            cv::rectangle(frame, face, cv::Scalar(0, 255, 0), 2);
            cv::putText(frame, name, face.tl(),
                        cv::FONT_HERSHEY_SIMPLEX, 1,
                        {0, 255, 0}, 2);
        }

        cv::imshow("Face Recognition", frame);

        if (cv::waitKey(1) == 27)
            break;
    }
}