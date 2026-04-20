#include "RecognitionThread.h"
#include "EventBus.h"
#include <iostream>

RecognitionThread::RecognitionThread(ThreadSafeQueue<cv::Mat>& queue)
    : queue_(queue) {
    recognizer_.loadModels(
        "/home/team22/IoT-Multi-Factor-Smart-Door-System/models/haarcascade_frontalface_default.xml",
        "/home/team22/IoT-Multi-Factor-Smart-Door-System/models/face_recognition.onnx");
    recognizer_.loadDatabase("database/embeddings.yml");
}

RecognitionThread::~RecognitionThread() { stop(); }

void RecognitionThread::start() {
    running_.store(true);
    thread_ = std::thread(&RecognitionThread::loop, this);
}

void RecognitionThread::stop() {
    running_.store(false);
    queue_.shutdown();
    if (thread_.joinable()) thread_.join();
}

void RecognitionThread::loop() {
    while (running_.load()) {
        auto maybeFrame = queue_.pop();   
        if (!maybeFrame) break;           

        auto events = recognizer_.process(*maybeFrame);

        for (auto& ev : events)
            EventBus::instance().publish(ev);  // CALLBACK

        drawOverlay(*maybeFrame, events);
        cv::imshow("FaceID Door Lock", *maybeFrame);
        if (cv::waitKey(1) == 27) {       // ESC Key
            queue_.shutdown();
            break;
        }
    }
    cv::destroyAllWindows();
}

void RecognitionThread::drawOverlay(cv::Mat& f,
                                    const std::vector<AccessEvent>& evs) {
    for (const auto& ev : evs) {
        if (!ev.faceRect) continue;
        cv::Scalar col = (ev.result == AuthResult::GRANTED)
                         ? cv::Scalar(0,255,0) : cv::Scalar(0,0,255);
        cv::rectangle(f, *ev.faceRect, col, 2);
        std::string lbl = ev.identity;
        if (ev.confidence > 0.f)
            lbl += " (" + std::to_string((int)(ev.confidence*100)) + "%)";
        cv::putText(f, lbl,
                    {ev.faceRect->x, ev.faceRect->y - 8},
                    cv::FONT_HERSHEY_SIMPLEX, 0.6, col, 2);
        if (ev.result == AuthResult::GRANTED)
            cv::putText(f, "UNLOCKED", {20,50},
                        cv::FONT_HERSHEY_SIMPLEX, 1.2,
                        cv::Scalar(0,255,0), 3);
    }
}
