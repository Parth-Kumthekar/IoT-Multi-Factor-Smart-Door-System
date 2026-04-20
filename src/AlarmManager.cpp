#include "AlarmManager.h"
#include <iostream>

/**
 * @brief Constructor for the AlarmManager.
 * @param oc Reference to the OutputController for hardware signaling.
 */
AlarmManager::AlarmManager(OutputController& oc) 
    : outputController_(oc), logger_(nullptr) {}

/**
 * @brief Destructor ensures a clean system state on exit.
 */
AlarmManager::~AlarmManager() {
    stop();
}

/**
 * @brief Starts the manager and connects the asynchronous logging service.
 */
void AlarmManager::start(AsyncLogger& logger) {
    logger_ = &logger;
    if (logger_) {
        logger_->log("SYSTEM", "AlarmManager initialized and active.");
    }
}

/**
 * @brief Shuts down the manager and silences active alarms.
 */
void AlarmManager::stop() {
    clearAlarm();
    if (logger_) {
        logger_->log("SYSTEM", "AlarmManager service stopped.");
    }
}

/**
 * @brief Activates the alarm state and triggers physical feedback.
 * * Uses a lock_guard to ensure that the state flag and the reason string 
 * are updated atomically, preventing race conditions with the Web API thread.
 */
void AlarmManager::triggerAlarm(const std::string& reason) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    // Efficiency check: skip if already in this state
    if (alarmActive_ && lastReason_ == reason) return;

    alarmActive_ = true;
    lastReason_ = reason;

    // Direct hardware interaction for lowest possible latency
    outputController_.setBuzzer(true);
    outputController_.setRedLed(true);

    if (logger_) {
        logger_->log("ALARM", "TRIGGERED: " + reason);
    }
}

/**
 * @brief Resets the alarm state and silences hardware outputs.
 */
void AlarmManager::clearAlarm() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    alarmActive_ = false;
    
    // Hardware Action: Silence the buzzer
    outputController_.setBuzzer(false);
    
    if (logger_) {
        logger_->log("ALARM", "CLEARED: System restored to normal operational state.");
    }
}

/**
 * @brief Thread-safe status check.
 * @return true if the system is currently in an alarm state.
 */
bool AlarmManager::isAlarmActive() const {
    // Atomic read of the flag
    return alarmActive_.load();
}