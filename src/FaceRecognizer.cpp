#include "FaceRecognizer.h"

FaceSystem::FaceSystem(ThreadSafeQueue<cv::Mat>& queue)
    : input_(queue)
{
    cascade_.load("models/haarcascade_frontalface_default.xml");
}

void FaceSystem::run() {

    int frameSkip = 0;

    while (true) {

        cv::Mat frame = input_.pop();

        if (frame.empty()) continue;

        // Resize for performance
        cv::resize(frame, frame, cv::Size(320, 240));

        // Convert to grayscale
        cv::Mat gray;
        cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);

        std::vector<cv::Rect> faces;

        
        if (frameSkip++ % 2 == 0) {
            cascade_.detectMultiScale(gray, faces, 1.2, 5);
        }

        // Draw results
        for (auto& face : faces) {
            cv::rectangle(frame, face, {0,255,0}, 2);
        }

        cv::imshow("FaceID", frame);

        if (cv::waitKey(1) == 27)
            break;
    }
}