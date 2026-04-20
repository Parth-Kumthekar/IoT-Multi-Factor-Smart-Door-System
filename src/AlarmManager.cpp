#include "AlarmManager.h"
#include <iostream>

/**
 * @brief Construct a new AlarmManager.
 * @details Initializes the reference to the OutputController and sets the logger to null.
 * @param oc Reference to the OutputController for hardware signaling.
 */
AlarmManager::AlarmManager(OutputController& oc) 
    : outputController_(oc), logger_(nullptr) {}

/**
 * @brief Cleans up the AlarmManager.
 * @details Calls stop() to ensure the alarm is cleared and hardware is reset before destruction.
 */
AlarmManager::~AlarmManager() {
    stop();
}

/**
 * @brief Activates the manager and assigns the logging service.
 * @param logger Reference to an active AsyncLogger instance.
 */
void AlarmManager::start(AsyncLogger& logger) {
    logger_ = &logger;
    if (logger_) {
        logger_->log("AlarmManager: Started and ready.");
    }
}

/**
 * @brief Stops the manager and resets state.
 * @details Clears any active alarms to turn off buzzers/LEDs and logs the shutdown event.
 */
void AlarmManager::stop() {
    clearAlarm();
    if (logger_) {
        logger_->log("AlarmManager: Shutting down.");
    }
}

/**
 * @brief Triggers a physical alarm and updates system state.
 * @details Performs a thread-safe update of the alarm state, stores the reason, 
 * activates hardware buzzer/LED via OutputController, and logs the event.
 * @param reason String describing the cause of the alarm trigger.
 */
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

/**
 * @brief Resets the alarm state and deactivates hardware signals.
 * @details Thread-safely clears the active flag and turns off the hardware buzzer.
 */
void AlarmManager::clearAlarm() {
    std::lock_guard<std::mutex> lock(mutex_);
    alarmActive_ = false;
    
    // Hardware Action
    outputController_.setBuzzer(false);
    
    if (logger_) {
        logger_->log("ALARM CLEARED: System restored to normal.");
    }
}

/**
 * @brief Thread-safe check of the current alarm status.
 * @return true if the alarm is currently active.
 */
bool AlarmManager::isAlarmActive() const {
    return alarmActive_;
}
