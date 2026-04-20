#ifndef ALARMMANAGER_H
#define ALARMMANAGER_H

#include <string>
#include <atomic>
#include <mutex>
#include "AsyncLogger.h"
#include "OutputController.hpp" // Required for hardware control

/**
 * @class AlarmManager
 * @brief Coordinates system-wide alarm states and hardware notifications.
 * * This class acts as a bridge between high-level logic triggers and the 
 * physical output hardware (via OutputController). It ensures thread-safe 
 * access to the alarm state and logs events asynchronously.
 */
class AlarmManager
{
public:
    /**
     * @brief Construct a new Alarm Manager object.
     * @param oc Reference to the OutputController used for physical signaling.
     * @note The OutputController must remain in scope for the lifetime of this object.
     */
    AlarmManager(OutputController& oc);

    /**
     * @brief Destroy the Alarm Manager object.
     * Ensures any active alarms are handled before cleanup.
     */
    ~AlarmManager();

    /**
     * @brief Initializes the manager and connects the logging service.
     * @param logger Reference to an active AsyncLogger for event recording.
     */
    void start(AsyncLogger& logger);

    /**
     * @brief Shuts down the alarm manager and stops active signaling.
     */
    void stop();

    /**
     * @brief Activates the alarm state.
     * @details Sets the internal state to active, triggers the hardware via 
     * the OutputController, and logs the reason provided.
     * @param reason A string description of why the alarm was triggered.
     */
    void triggerAlarm(const std::string& reason);

    /**
     * @brief Resets the alarm state to normal.
     * Clears hardware signals and updates the status.
     */
    void clearAlarm();

    /**
     * @brief Checks the current status of the alarm.
     * @return true if the alarm is currently triggered, false otherwise.
     */
    bool isAlarmActive() const;

private:
    /// Reference to the hardware controller dependency.
    OutputController& outputController_;
    
    /// Pointer to the logger, assigned during start().
    AsyncLogger* logger_ = nullptr;

    /// Thread-safe flag indicating if the alarm is currently active.
    std::atomic<bool> alarmActive_{false};
    
    /// Mutex protecting access to the lastReason_ string.
    mutable std::mutex mutex_;
    
    /// Stores the most recent reason for an alarm trigger.
    std::string lastReason_ = "unknown";
};

#endif
