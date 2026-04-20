#include "AlarmManager.h"
#include <iostream>

AlarmManager::AlarmManager(OutputController& oc) 
    : outputController_(oc), logger_(nullptr) {}

AlarmManager::~AlarmManager() {
    stop();
}

void AlarmManager::start(AsyncLogger& logger) {
    logger_ = &logger;
    if (logger_) {
        logger_->log("AlarmManager: Started and ready.");
    }
}

void AlarmManager::stop() {
    clearAlarm();
    if (logger_) {
        logger_->log("AlarmManager: Shutting down.");
    }
}

void AlarmManager::triggerAlarm(const std::string& reason) {
    std::lock_guard<std::mutex> lock(mutex_);
    alarmActive_ = true;
    lastReason_ = reason;

    // Hardware Action
    outputController_.setBuzzer(true);
    outputController_.setRedLed(true);

    if (logger_) {
        logger_->log("ALARM TRIGGERED: " + reason);
    }
}

void AlarmManager::clearAlarm() {
    std::lock_guard<std::mutex> lock(mutex_);
    alarmActive_ = false;
    
    // Hardware Action
    outputController_.setBuzzer(false);
    
    if (logger_) {
        logger_->log("ALARM CLEARED: System restored to normal.");
    }
}

bool AlarmManager::isAlarmActive() const {
    return alarmActive_;
}
