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
 * @brief Logic engine implementing the security state machine.
 */
class DoorAlarmFSM
{
public:
    using Clock = std::chrono::steady_clock;
    using Ms = std::chrono::milliseconds;

    enum class State
    {
        Disarmed,
        ArmedIdle,
        PendingVerification,
        AuthorizedEntry,
        AlarmActive,
        Fault
    };

    DoorAlarmFSM(AlarmManager& alarmManager, AsyncLogger& logger);

    // Main entry point for state changes
    void handleEvent(const Event& event);
    
    // Configuration
    void setAuthorizationWindow(Ms window);
    
    // --- Thread-safe Accessors ---
    State getState() const;
    bool isDoorOpen() const;
    bool isAlarmActive() const;
    
    // Returns the point in time when the alarm will trigger if not authorized
    std::optional<Clock::time_point> getVerificationDeadline() const;

    // Static helper for external classes (API/System) to get state names
    static std::string toString(State state);

    // Public wrapper that handles locking before logging status
    void printStatus();

private:
    // Internal handlers (Assumes mutex is already locked)
    void handleArm(const std::string& source);
    void handleDisarm(const std::string& source);
    void handleDoorOpened(const std::string& source);
    void handleDoorClosed(const std::string& source);
    void handleAuthorization(const Event& event);
    void handleVerificationTimeout(const std::string& source);
    
    void clearAuthorizationWindow();
    void printStatusInternal(); 

private:
    State state_;
    bool doorOpen_;
    AlarmManager& alarmManager_;
    AsyncLogger& logger_;
    
    // Mutable allows const-qualified getters to perform thread-safe locking
    mutable std::mutex mutex_; 
    
    Ms authorizationWindow_;
    std::optional<Clock::time_point> verificationDeadline_;
};

#endif