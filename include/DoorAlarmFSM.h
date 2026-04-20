#ifndef DOORALARMFSM_H
#define DOORALARMFSM_H

#include "AlarmManager.h"
#include "AsyncLogger.h"
#include "Event.h"
#include <chrono>
#include <mutex>
#include <optional>
#include <string>

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

    // --- Core Logic ---
    void handleEvent(const Event& event);
    void setAuthorizationWindow(Ms window);
    
    // --- Getters (Thread-Safe) ---
    State getState() const;
    std::string getStateString() const; // For Web API JSON
    bool isDoorOpen() const;
    bool isAlarmActive() const;
    std::optional<Clock::time_point> getVerificationDeadline() const;

    // --- Static Helpers ---
    static std::string toString(State state);
    void printStatus();

private:
    // Event Handlers
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
    // Order matches constructor initialization list
    State state_;
    bool doorOpen_;
    AlarmManager& alarmManager_;
    AsyncLogger& logger_;

    mutable std::mutex mutex_; // mutable allows locking inside const getters
    Ms authorizationWindow_;
    std::optional<Clock::time_point> verificationDeadline_;
};

#endif