#include <opencv2/opencv.hpp>
#include <iostream>
#include <filesystem>

namespace fs = std::filesystem;

int main() {
    // ---------------- Determine project root ----------------
    fs::path exe_path = fs::current_path();
    fs::path project_root = exe_path.parent_path().parent_path(); // adjust for build/Release
    std::cout << "Current working directory: " << exe_path << std::endl;
    std::cout << "Project root: " << project_root << std::endl;

    // ---------------- Ensure dataset folder exists ----------------
    fs::path dataset_folder = project_root / "dataset";
    if (!fs::exists(dataset_folder)) {
        fs::create_directory(dataset_folder);
        std::cout << "Created dataset folder at: " << dataset_folder << std::endl;
    } else {
        std::cout << "Dataset folder exists at: " << dataset_folder << std::endl;
    }

    // ---------------- Open webcam ----------------
    cv::VideoCapture cap(0);
    if (!cap.isOpened()) {
        std::cerr << "Camera not working!" << std::endl;
        return -1;
    }

    int count = 0;
    std::string name;
    std::cout << "Enter your name for dataset images: ";
    std::cin >> name;

    std::cout << "Press 'c' to capture an image, 'q' to quit\n";

    while (true) {
        cv::Mat frame;
        cap >> frame;
        if (frame.empty()) continue;

        cv::imshow("Capture Dataset", frame);

        char key = static_cast<char>(cv::waitKey(1));
        if (key == 'c') {
            fs::path filename = dataset_folder / (name + "_" + std::to_string(count) + ".jpg");
            if (cv::imwrite(filename.string(), frame)) {
                std::cout << "Saved image: " << filename << std::endl;
                count++;
            } else {
                std::cerr << "Failed to save image to: " << filename << std::endl;
            }
        }
        if (key == 'q') break;
    }

    cap.release();
    cv::destroyAllWindows();
    std::cout << "Finished capturing " << count << " images." << std::endl;

    return 0;
}