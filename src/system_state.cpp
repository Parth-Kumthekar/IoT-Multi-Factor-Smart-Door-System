#include "system_state.hpp"

void SystemState::setArmed(bool v) {
    std::lock_guard<std::mutex> lock(mutex_);
    armed_ = v;
}

bool SystemState::isArmed() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return armed_;
}

void SystemState::setDoorOpen(bool v) {
    std::lock_guard<std::mutex> lock(mutex_);
    doorOpen_ = v;
}

bool SystemState::isDoorOpen() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return doorOpen_;
}

void SystemState::setAlarmActive(bool v) {
    std::lock_guard<std::mutex> lock(mutex_);
    alarmActive_ = v;
}

bool SystemState::isAlarmActive() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return alarmActive_;
}

void SystemState::setLastAuth(const std::string& user, long long timeMs) {
    std::lock_guard<std::mutex> lock(mutex_);
    lastAuthUser_ = user;
    lastAuthTime_ = timeMs;
}

std::string SystemState::getLastAuthUser() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return lastAuthUser_;
}

long long SystemState::getLastAuthTime() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return lastAuthTime_;
}

void SystemState::setLastIntrusionTime(long long timeMs) {
    std::lock_guard<std::mutex> lock(mutex_);
    lastIntrusionTime_ = timeMs;
}

long long SystemState::getLastIntrusionTime() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return lastIntrusionTime_;
}

std::string SystemState::toJson() const {
    std::lock_guard<std::mutex> lock(mutex_);

    std::ostringstream oss;
    oss << "{"
        << "\"armed\":" << (armed_ ? "true" : "false") << ","
        << "\"door_open\":" << (doorOpen_ ? "true" : "false") << ","
        << "\"alarm_active\":" << (alarmActive_ ? "true" : "false") << ","
        << "\"last_auth_user\":\"" << lastAuthUser_ << "\","
        << "\"last_auth_time\":" << lastAuthTime_ << ","
        << "\"last_intrusion_time\":" << lastIntrusionTime_
        << "}";

    return oss.str();
}