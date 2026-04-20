#ifndef DOORALARMFSM_H
#define DOORALARMFSM_H

#include "AlarmManager.h"
#include "AsyncLogger.h"
#include "Event.h"
#include <chrono>
#include <mutex>
#include <optional>
#include <string>

/**
 * @class DoorAlarmFSM
 * @brief Finite State Machine (FSM) controlling the door security logic.
 * * This class manages transitions between security states (Disarmed, Armed, etc.)
 * based on input events. It coordinates with the AlarmManager for hardware 
 * signaling and the AsyncLogger for event history.
 */
class DoorAlarmFSM
{
public:
    /// Type alias for the steady clock used for timeouts.
    using Clock = std::chrono::steady_clock;
    /// Type alias for millisecond durations.
    using Ms = std::chrono::milliseconds;

    /**
     * @enum State
     * @brief Represents the various operational states of the security system.
     */
    enum class State
    {
        Disarmed,            ///< System is inactive; door movement is ignored.
        ArmedIdle,           ///< System is active and monitoring for door events.
        PendingVerification, ///< Door opened; waiting for authorized UID within window.
        AuthorizedEntry,     ///< UID verified; door may be opened without alarm.
        AlarmActive,         ///< Security breach detected; alarm is sounding.
        Fault                ///< System encountered an inconsistent hardware state.
    };

    /**
     * @brief Construct a new Door Alarm FSM object.
     * @param alarmManager Reference to the hardware alarm controller.
     * @param logger Reference to the asynchronous logging service.
     */
    DoorAlarmFSM(AlarmManager& alarmManager, AsyncLogger& logger);

    // --- Core Logic ---
    
    /**
     * @brief Primary entry point for all system inputs.
     * @param event The event (Door, Card, System) to be processed.
     */
    void handleEvent(const Event& event);

    /**
     * @brief Configures the grace period for UID verification.
     * @param window Duration in milliseconds.
     */
    void setAuthorizationWindow(Ms window);
    
    // --- Getters (Thread-Safe) ---

    /** @brief Returns the current state of the FSM. */
    State getState() const;

    /** @brief Returns a string representation of the current state for API/JSON usage. */
    std::string getStateString() const;

    /** @brief Returns true if the door sensor is currently reporting an open state. */
    bool isDoorOpen() const;

    /** @brief Returns true if the alarm is currently active. */
    bool isAlarmActive() const;

    /** @brief Returns the timestamp of the current verification timeout, if active. */
    std::optional<Clock::time_point> getVerificationDeadline() const;

    // --- Static Helpers ---

    /**
     * @brief Converts a State enum to a human-readable string.
     * @param state The state to convert.
     * @return std::string The name of the state.
     */
    static std::string toString(State state);

    /** @brief Public wrapper to print the current FSM status to console. */
    void printStatus();

private:
    // Event Handlers (Internal transition logic)
    void handleArm(const std::string& source);
    void handleDisarm(const std::string& source);
    void handleDoorOpened(const std::string& source);
    void handleDoorClosed(const std::string& source);
    void handleAuthorization(const Event& event);
    void handleVerificationTimeout(const std::string& source);
    
    // Internal Helpers
    void clearAuthorizationWindow();
    void printStatusInternal(); 

private:
    /// Current operational state.
    State state_;
    
    /// Cached physical state of the door sensor.
    bool doorOpen_;
    
    /// Dependency: Hardware alarm interface.
    AlarmManager& alarmManager_;
    
    /// Dependency: System logger.
    AsyncLogger& logger_;

    /// Mutex protecting internal state; mutable to allow locking in const getters.
    mutable std::mutex mutex_;

    /// Configured duration for the verification window.
    Ms authorizationWindow_;

    /// The deadline for UID verification when in PendingVerification state.
    std::optional<Clock::time_point> verificationDeadline_;
};

#endif
