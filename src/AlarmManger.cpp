#include "AlarmManager.h"
#include "EmailAlert.h" // Your email sending class
#include <thread>

void AlarmManager::triggerAlarm(const std::string& reason) {
    if (alarmActive_) return;
    alarmActive_ = true;

    // 1. Hardware Action: Turn on the Buzzer/Siren immediately
    outputController_.setBuzzer(true);

    // 2. External Action: Send Email Alert in a background thread
    // We detach so the main system doesn't "freeze" while talking to Gmail
    std::thread([reason]() {
        EmailAlert email;
        email.send("SECURITY ALERT", "Intrusion detected: " + reason);
    }).detach();

    logger_.log("ALARM: Buzzer activated and Alert Email dispatched.");
}

void AlarmManager::clearAlarm() {
    alarmActive_ = false;
    outputController_.setBuzzer(false);
    logger_.log("ALARM: System cleared/silenced.");
}