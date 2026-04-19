#include "CameraThread.h"
#include <iostream>
#include <stdexcept>
#include <string>
#include <cstdio>
#include <memory>
#include <array>


static std::string execCommand(const std::string& cmd) {
    std::array<char, 256> buf;
    std::string result;
    std::unique_ptr<FILE, decltype(&pclose)>
        pipe(popen(cmd.c_str(), "r"), pclose);
    if (!pipe) return "";
    while (fgets(buf.data(), buf.size(), pipe.get()))
        result += buf.data();
    return result;
}


static std::string discoverCameraPath() {
    std::string out = execCommand("rpicam-hello --list-cameras 2>&1");
    auto start = out.find('(');
    auto end   = out.find(')', start);
    if (start != std::string::npos && end != std::string::npos)
        return out.substr(start + 1, end - start - 1);
    return "";  
}


static std::string buildPipeline(int w, int h, int fps,
                                 const std::string& cameraPath = "") {
    std::string src = "libcamerasrc";
    if (!cameraPath.empty())
        src += " camera-name=\"" + cameraPath + "\"";

    return src +
           " ! video/x-raw"
           ",colorimetry=bt709"
           ",format=NV12"
           ",width="      + std::to_string(w) +
           ",height="     + std::to_string(h) +
           ",framerate="  + std::to_string(fps) + "/1"
           " ! queue leaky=downstream max-size-buffers=2"
           " ! videoconvert"
           " ! video/x-raw,format=BGR"
           " ! queue leaky=downstream max-size-buffers=1"
           " ! appsink sync=false drop=true max-buffers=1";
}



CameraThread::CameraThread(ThreadSafeQueue<cv::Mat>& queue,
                           int dev, int w, int h)
    : queue_(queue), deviceIndex_(dev), width_(w), height_(h) {}

CameraThread::~CameraThread() { stop(); }

void CameraThread::start() {
    running_.store(true);
    thread_ = std::thread(&CameraThread::loop, this);
}

void CameraThread::stop() {
    running_.store(false);
    queue_.shutdown();
    if (thread_.joinable()) thread_.join();
}

void CameraThread::loop() {
    cv::VideoCapture cap;

    
    std::string camPath = discoverCameraPath();
    if (!camPath.empty()) {
        std::string pipeline = buildPipeline(width_, height_, 30, camPath);
        std::cout << "[Camera] Trying pipeline:\n  " << pipeline << '\n';
        cap.open(pipeline, cv::CAP_GSTREAMER);
    }

    
    if (!cap.isOpened()) {
        std::string pipeline = buildPipeline(width_, height_, 30);
        std::cout << "[Camera] Trying pipeline (no camera-name):\n  "
                  << pipeline << '\n';
        cap.open(pipeline, cv::CAP_GSTREAMER);
    }

    
    if (!cap.isOpened()) {
        std::cerr << "[Camera] GStreamer failed — trying V4L2 index "
                  << deviceIndex_ << '\n';
        cap.open(deviceIndex_, cv::CAP_V4L2);
        if (cap.isOpened()) {
            cap.set(cv::CAP_PROP_FRAME_WIDTH,  width_);
            cap.set(cv::CAP_PROP_FRAME_HEIGHT, height_);
        }
    }

    if (!cap.isOpened())
        throw std::runtime_error(
            "[Camera] All capture methods failed.\n"
            "  Check: gst-inspect-1.0 libcamerasrc\n"
            "  Check: rpicam-hello --list-cameras\n"
            "  Check: OpenCV built with -DWITH_GSTREAMER=ON");

    std::cout << "[Camera] Capture open at "
              << width_ << 'x' << height_ << '\n';

    while (running_.load()) {
        cv::Mat frame;
        cap >> frame;
        if (frame.empty()) {
            std::cerr << "[Camera] Empty frame — "
                         "camera may have been disconnected\n";
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            continue;
        }
        // Bounded push: drop oldest frames if consumer is too slow
        queue_.pushBounded(std::move(frame), 4);
    }
}