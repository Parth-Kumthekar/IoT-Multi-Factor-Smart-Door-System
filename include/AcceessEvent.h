#pragma once
#include <string>
#include <chrono>
#include <optional>
#include <opencv2/core.hpp>

enum class AuthMethod  { FACE_ID, NFC, OVERRIDE, MANUAL, SYSTEM };
enum class AuthResult  { GRANTED, DENIED, OVERRIDE };

struct AccessEvent {
    std::chrono::system_clock::time_point timestamp;
    AuthMethod   method;
    AuthResult   result;
    std::string  identity;      // name, UID string, or "Unknown"
    float        confidence{0}; 
    std::string  note;
    std::optional<cv::Rect> faceRect; 

    
    static AccessEvent granted(AuthMethod m, const std::string& id,
                               float conf = 1.f,
                               const std::string& note = "") {
        return { now(), m, AuthResult::GRANTED, id, conf, note, {} };
    }
    static AccessEvent denied(AuthMethod m, const std::string& id,
                              float conf = 0.f,
                              const std::string& note = "") {
        return { now(), m, AuthResult::DENIED, id, conf, note, {} };
    }
    static AccessEvent override_(const std::string& by,
                                 const std::string& note = "") {
        return { now(), AuthMethod::OVERRIDE, AuthResult::OVERRIDE,
                 by, 1.f, note, {} };
    }

    static std::string methodStr(AuthMethod m) {
        switch(m) {
            case AuthMethod::FACE_ID:  return "Face ID";
            case AuthMethod::NFC:      return "NFC";
            case AuthMethod::OVERRIDE: return "Override";
            case AuthMethod::MANUAL:   return "Manual";
            case AuthMethod::SYSTEM:   return "System";
        }
        return "Unknown";
    }
    static std::string resultStr(AuthResult r) {
        switch(r) {
            case AuthResult::GRANTED:  return "Granted";
            case AuthResult::DENIED:   return "Denied";
            case AuthResult::OVERRIDE: return "Override";
        }
        return "Unknown";
    }

private:
    static std::chrono::system_clock::time_point now() {
        return std::chrono::system_clock::now();
    }
};