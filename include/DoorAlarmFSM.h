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
 * @brief Logic engine for the security system implemented as a Finite State Machine.
 * * This class manages the transitions between security states (e.g., Armed, Pending, Alarm).
 * It ensures that all door interactions follow the security protocol and handles 
 * timing-sensitive verification windows using steady clock references.
 */
class DoorAlarmFSM
{
public:
    using Clock = std::chrono::steady_clock;
    using Ms = std::chrono::milliseconds;

    /**
     * @enum State
     * @brief Internal states of the security system.
     */
    enum class State
    {
        Disarmed,            ///< System is off; door movement is ignored.
        ArmedIdle,           ///< System is active; waiting for sensors.
        PendingVerification, ///< Door opened; waiting for valid NFC/Auth.
        AuthorizedEntry,     ///< Valid ID presented; door access granted.
        AlarmActive,         ///< Security breach detected; alarm signaling.
        Fault                ///< System error state for fail-safe operation.
    };

    /**
     * @brief Constructs the FSM with required management dependencies.
     * @param alarmManager Reference to the hardware alarm controller.
     * @param logger Reference to the asynchronous logging system.
     */
    DoorAlarmFSM(AlarmManager& alarmManager, AsyncLogger& logger);

    /**
     * @brief Processes incoming system events and triggers state transitions.
     * @param event The structured event (NFC scan, Sensor trigger, Timer).
     */
    void handleEvent(const Event& event);

    /**
     * @brief Configures the grace period for user verification.
     * @param window Time in milliseconds before an alarm triggers.
     */
    void setAuthorizationWindow(Ms window);
    
    /** @brief Retrieves the current state (Thread-safe). */
    State getState() const;

    /** @brief Returns a string representation of the state for JSON/Web APIs. */
    std::string getStateString() const;

    /** @brief Returns true if the physical door sensor reports 'Open'. */
    bool isDoorOpen() const;

    /** @brief Checks if the FSM is currently in the AlarmActive state. */
    bool isAlarmActive() const;

    /** @brief Gets the timestamp when the current pending window will expire. */
    std::optional<Clock::time_point> getVerificationDeadline() const;

    /** @brief Converts a State enum to a human-readable string. */
    static std::string toString(State state);

    /** @brief Logs the current status to the console/logger. */
    void printStatus();

private:
    // Event Handlers for specific transition logic
    void handleArm(const std::string& source);
    void handleDisarm(const std::string& source);
    void handleDoorOpened(const std::string& source);
    void handleDoorClosed(const std::string& source);
    void handleAuthorization(const Event& event);
    void handleVerificationTimeout(const std::string& source);
    
    void clearAuthorizationWindow();
    void printStatusInternal(); 

private:
    /** @brief Current operational state of the FSM. */
    State state_;

    /** @brief Cached status of the door sensor. */
    bool doorOpen_;

    /** @brief Reference to the AlarmManager (Dependency Injection). */
    AlarmManager& alarmManager_;

    /** @brief Reference to the Logger (Dependency Injection). */
    AsyncLogger& logger_;

    /** @brief Mutex protecting state and variables during concurrent access. */
    mutable std::mutex mutex_;

    /** @brief Duration allowed for verification after door entry. */
    Ms authorizationWindow_;

    /** @brief The exact time point when the verification window closes. */
    std::optional<Clock::time_point> verificationDeadline_;
};

#endif