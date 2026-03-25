#include <opencv2/opencv.hpp>
#include <opencv2/dnn.hpp>
#include <filesystem>
#include <iostream>
#include <map>

namespace fs = std::filesystem;

int main() {

    // PATH SETUP
    fs::path exe_path = fs::current_path();
    fs::path project_root = exe_path.parent_path().parent_path();

    fs::path dataset_folder  = project_root / "dataset";
    fs::path model_folder    = project_root / "models";
    fs::path database_folder = project_root / "database";

    fs::path haar_path  = model_folder / "haarcascade_frontalface_default.xml";
    fs::path onnx_path  = model_folder / "face_recognition.onnx";
    fs::path output_yml = database_folder / "embeddings.yml";

    std::cout << "Project root: " << project_root << std::endl;
    std::cout << "ONNX path: " << onnx_path << std::endl;

    // CREATE DATABASE FOLDER
    if (!fs::exists(database_folder)) {
        fs::create_directory(database_folder);
        std::cout << "Created database folder.\n";
    }

    //LOAD HAAR CASCADE
    cv::CascadeClassifier faceCascade;
    if (!faceCascade.load(haar_path.string())) {
        std::cerr << "Failed to load Haar cascade!\n";
        return -1;
    }

    //  LOAD ONNX MODEL
    cv::dnn::Net net = cv::dnn::readNetFromONNX(onnx_path.string());
    if (net.empty()) {
        std::cerr << "Failed to load ONNX model!\n";
        return -1;
    }

    net.setPreferableBackend(cv::dnn::DNN_BACKEND_OPENCV);
    net.setPreferableTarget(cv::dnn::DNN_TARGET_CPU);

    std::map<std::string, cv::Mat> database;

    // CHECK DATASET 
    if (!fs::exists(dataset_folder))  {
        std::cerr << "Dataset folder NOT found!\n";
        return -1;
    }

    // PROCESS IMAGES
    for(auto& entry : fs::directory_iterator(dataset_folder)) {

        if (!entry.is_regular_file()) continue;

        std::string filepath = entry.path().string();
        std::string filename = entry.path().stem().string();

        std::cout << "\nProcessing: " << filepath << std::endl;

        cv::Mat img = cv::imread(filepath);
        if (img.empty()) {
            std::cerr << "Failed to load image!\n";
            continue;
        }

       
        std::vector<cv::Rect> faces;
        faceCascade.detectMultiScale(img, faces);

        cv::Mat face;
        if (!faces.empty()) {
            face = img(faces[0]).clone();
            std::cout << "Face detected\n";
        } else {
            face = img.clone();
            std::cout << "No face detected, using full image\n";
        }

        // PREPROCESSING 
        cv::resize(face, face, cv::Size(112, 112));

        if (face.channels() == 1) {
            cv::cvtColor(face, face, cv::COLOR_GRAY2BGR);
        }

        cv::cvtColor(face, face, cv::COLOR_BGR2RGB);
        face.convertTo(face, CV_32F, 1.0 / 255.0);

        std::cout << "Face shape: "
                  << face.rows << "x" << face.cols
                  << " channels: " << face.channels() << std::endl;

        //CREATE BLOB 
        cv::Mat blob = cv::dnn::blobFromImage(
            face,
            1.0,
            cv::Size(112, 112),
            cv::Scalar(0, 0, 0),
            false,
            false
        );

        // Debug blob dimensions
        std::cout << "Blob dims: ";
        for (int i = 0; i < blob.dims; i++) {
            std::cout << blob.size[i] << " ";
        }
        std::cout << std::endl;

        blob = blob.reshape(1, {1, 3, 112, 112});

        std::cout << "Blob dims: ";
        for (int i = 0; i < blob.dims; i++) {
            std::cout << blob.size[i] << " ";
        }
        std::cout << std::endl;

        // FORWARD PASS
        net.setInput(blob);

        cv::Mat embedding;
        try {
            embedding = net.forward();
        } catch (const cv::Exception& e) {
            std::cerr << "ONNX Forward Error:\n" << e.what() << std::endl;
            continue;
        }

        cv::normalize(embedding, embedding);

        database[filename] = embedding.clone();
        std::cout << "Embedding stored\n";
    }

    // YAML
    cv::FileStorage fs_out(output_yml.string(), cv::FileStorage::WRITE);

    if (!fs_out.isOpened()) {
        std::cerr << "Failed to open YAML file for writing!\n";
        return -1;
    }

    for (auto& [name, emb] : database) {
        fs_out << name << emb;
    }

    fs_out.release();

    std::cout << "\nDatabase saved at: " << output_yml << std::endl;
    std::cout << "Total entries: " << database.size() << std::endl;

    return 0;
}