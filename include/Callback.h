
#pragma once
#include <functional>
#include <string>
#include <opencv2/core.hpp>   


using UnlockCallback = std::function<void(const std::string& name)>;


struct RecognitionResult {
    std::string name;        
    float       confidence;  
    cv::Rect    faceRect;    // bounding box in frame coordinates
    bool        unlocked;    // true this frame if door was triggered
};

using FrameCallback = std::function<void(const RecognitionResult&)>;