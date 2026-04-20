#include "AlarmManager.h"
#include "EmailAlert.h"
#include <thread>

/**
 * Constructor
 * Note: Must initialize the outputController_ reference.
 */
AlarmManager::AlarmManager(OutputController& oc)
    : outputController_(oc), alarmActive_(false) 
{
}

void AlarmManager::start(AsyncLogger& logger) {
    logger_ = &logger;
}

void AlarmManager::stop() {
    clearAlarm();
}

void AlarmManager::triggerAlarm(const std::string& reason) {
    // atomics exchange: if it was already true, we return. 
    // This prevents spawning multiple email threads for one alarm.
    if (alarmActive_.exchange(true)) {
        return; 
    }

    // 1. Hardware Action: Siren/Buzzer ON
    outputController_.setBuzzer(true);

    // 2. Logging (Safe check for pointer)
    if (logger_) {
        logger_->log("ALARM triggered: " + reason);
    }

    // 3. Email Alert: Detached thread so the main logic doesn't wait for SMTP
    std::thread([reason]() {
        try {
            EmailAlert email;
            email.send("CRITICAL: Smart Door Security Alert", 
                       "An intrusion event was detected.\nReason: " + reason);
        } catch (...) {
            // Silently fail inside thread to prevent app crash if internet is down
        }
    }).detach();
}

void AlarmManager::clearAlarm() {
    // If it was already false, do nothing
    if (!alarmActive_.exchange(false)) {
        return;
    }

    // 1. Hardware Action: Siren/Buzzer OFF
    outputController_.setBuzzer(false);

    // 2. Logging
    if (logger_) {
        logger_->log("ALARM: System cleared and silenced.");
    }
}

bool AlarmManager::isAlarmActive() const {
    return alarmActive_.load();
}