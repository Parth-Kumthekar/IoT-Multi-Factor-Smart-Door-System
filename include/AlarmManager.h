#ifndef ALARMMANAGER_H
#define ALARMMANAGER_H

#include <string>
#include <atomic>
#include <mutex>
#include "AsyncLogger.h"
#include "OutputController.hpp"

/**
 * @class AlarmManager
 * @brief Handles system-wide alarm states and hardware signaling.
 * * This class coordinates the activation of the alarm system, interacting with 
 * the OutputController for physical feedback and the AsyncLogger for event 
 * recording. It is designed to be thread-safe for use in real-time environments.
 */
class AlarmManager
{
public:
    /**
     * @brief Constructs an AlarmManager with a reference to the hardware controller.
     * @param oc Reference to the OutputController responsible for physical signaling.
     */
    AlarmManager(OutputController& oc);

    /**
     * @brief Cleans up resources and ensures the alarm state is neutralized.
     */
    ~AlarmManager();

    /**
     * @brief Initializes the manager and connects the logging service.
     * @param logger Reference to the asynchronous logging system.
     */
    void start(AsyncLogger& logger);

    /**
     * @brief Safely shuts down the alarm manager and stops active signaling.
     */
    void stop();

    /**
     * @brief Triggers the alarm state and logs the specific event.
     * @param reason A description of the event that caused the alarm trigger.
     */
    void triggerAlarm(const std::string& reason);

    /**
     * @brief Resets the alarm state and updates the hardware output.
     */
    void clearAlarm();

    /**
     * @brief Checks the current status of the alarm.
     * @return true if an alarm is currently active, false otherwise.
     */
    bool isAlarmActive() const;

private:
    /** @brief Reference to the hardware interface for driving outputs. */
    OutputController& outputController_;
    
    /** @brief Pointer to the asynchronous logger for non-blocking I/O. */
    AsyncLogger* logger_ = nullptr;

    /** @brief Atomic flag for lock-free status checks in high-frequency loops. */
    std::atomic<bool> alarmActive_{false};

    /** @brief Mutex to protect access to the alarm reason string. */
    mutable std::mutex mutex_;

    /** @brief Stores the description of the most recent alarm event. */
    std::string lastReason_ = "unknown";
};

#endif