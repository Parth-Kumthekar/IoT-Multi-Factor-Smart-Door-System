#ifndef ALARMMANAGER_H
#define ALARMMANAGER_H

#include <string>
#include <atomic>
#include <mutex>
#include "AsyncLogger.h"
#include "OutputController.hpp" // Required for hardware control

class AlarmManager
{
public:
    // Constructor now requires the hardware controller
    AlarmManager(OutputController& oc);

    ~AlarmManager();

    // Setup and Shutdown
    void start(AsyncLogger& logger);
    void stop();

    // Core Logic
    void triggerAlarm(const std::string& reason);
    void clearAlarm();
    bool isAlarmActive() const;

private:
    // Dependencies
    OutputController& outputController_;
    AsyncLogger* logger_ = nullptr;

    // State
    std::atomic<bool> alarmActive_{false};
    mutable std::mutex mutex_;
    std::string lastReason_ = "unknown";
};

#endif