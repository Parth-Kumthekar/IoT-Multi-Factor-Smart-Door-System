

#include <opencv2/opencv.hpp>
#include <filesystem>
#include <iostream>
#include <string>

namespace fs = std::filesystem;

int main() {
    cv::VideoCapture cap(0, cv::CAP_V4L2);
    if (!cap.isOpened()) {
        std::cerr << "Cannot open camera\n";
        return 1;
    }
    cap.set(cv::CAP_PROP_FRAME_WIDTH,  640);
    cap.set(cv::CAP_PROP_FRAME_HEIGHT, 480);

    cv::CascadeClassifier cascade;
    if (!cascade.load("models/haarcascade_frontalface_default.xml")) {
        std::cerr << "Cannot load cascade\n";
        return 1;
    }

    while (true) {
        std::string name;
        std::cout << "Enter person name (or 'exit'): ";
        std::cin >> name;
        if (name == "exit") break;

        fs::create_directories("dataset/" + name);
        int count = 0;
        std::cout << "Press 'c' to capture, 'q' when done with this person.\n"; //   Enter name, press 'c' to capture ~20 frames, 'q' to finish person, 'x' to exit.

        while (true) {
            cv::Mat frame;
            cap >> frame;
            if (frame.empty()) continue;

            cv::Mat gray;
            cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);
            std::vector<cv::Rect> faces;
            cascade.detectMultiScale(gray, faces, 1.1, 5, 0, cv::Size(80,80));

            for (const auto& r : faces)
                cv::rectangle(frame, r, cv::Scalar(0,255,0), 2);

            cv::putText(frame, "Captured: " + std::to_string(count),
                        {10, 30}, cv::FONT_HERSHEY_SIMPLEX, 0.8,
                        cv::Scalar(255,255,0), 2);
            cv::imshow("Capture — " + name, frame);

            int key = cv::waitKey(30);
            if (key == 'c' && !faces.empty()) {
                std::string path = "dataset/" + name + "/" +
                                   std::to_string(count++) + ".jpg";
                cv::imwrite(path, frame(faces[0]));
                std::cout << "  Saved " << path << "\n";
            } else if (key == 'q') {
                break;
            }
        }
        cv::destroyAllWindows();
        std::cout << "Saved " << count << " images for " << name << "\n";
    }
    return 0;
}