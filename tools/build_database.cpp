

#include <opencv2/opencv.hpp>
#include <opencv2/dnn.hpp>
#include <filesystem>
#include <iostream>
#include <vector>
#include <string>

namespace fs = std::filesystem;

static std::vector<float> getEmbedding(cv::dnn::Net& net,
                                       const cv::Mat& face) {
    cv::Mat resized;
    cv::resize(face, resized, cv::Size(112, 112));
    cv::Mat blob = cv::dnn::blobFromImage(resized, 1.0/128.0,
                                          cv::Size(112,112),
                                          cv::Scalar(127.5,127.5,127.5),
                                          true, false);
    net.setInput(blob);
    cv::Mat out = net.forward();
    return std::vector<float>(out.begin<float>(), out.end<float>());
}

int main() {
    cv::dnn::Net net = cv::dnn::readNetFromONNX("models/face_recognition.onnx");
    if (net.empty()) {
        std::cerr << "Cannot load ONNX model\n";
        return 1;
    }
    net.setPreferableBackend(cv::dnn::DNN_BACKEND_DEFAULT);
    net.setPreferableTarget(cv::dnn::DNN_TARGET_CPU);

    fs::create_directories("database");
    cv::FileStorage fs_out("database/embeddings.yml",
                           cv::FileStorage::WRITE);
    fs_out << "people" << "[";

    for (const auto& personDir : fs::directory_iterator("dataset")) {
        if (!personDir.is_directory()) continue;
        std::string name = personDir.path().filename().string();
        std::cout << "Processing: " << name << "\n";

        std::vector<std::vector<float>> embeddings;
        for (const auto& img : fs::directory_iterator(personDir.path())) {
            cv::Mat face = cv::imread(img.path().string());
            if (face.empty()) continue;
            embeddings.push_back(getEmbedding(net, face));
        }
        if (embeddings.empty()) continue;

        // Average the embeddings for a person
        size_t dim = embeddings[0].size();
        std::vector<float> avg(dim, 0.f);
        for (const auto& e : embeddings)
            for (size_t i = 0; i < dim; ++i) avg[i] += e[i];
        for (auto& v : avg) v /= (float)embeddings.size();

        cv::Mat embMat(1, (int)dim, CV_32F, avg.data());
        fs_out << "{" << "name" << name << "embedding" << embMat << "}";
        std::cout << "  -> " << embeddings.size() << " images averaged\n";
    }

    fs_out << "]";
    fs_out.release();
    std::cout << "Database written to database/embeddings.yml\n";
    return 0;
}