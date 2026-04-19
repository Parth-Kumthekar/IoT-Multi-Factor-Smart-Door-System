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

    void handleEvent(const Event& event);
    void setAuthorizationWindow(Ms window);
    void printStatus(); // Moved to public so DoorAlarmSystem can call it

    State getState() const;
    bool isDoorOpen() const;
    bool isAlarmActive() const;
    std::optional<Clock::time_point> getVerificationDeadline() const;

    static std::string toString(State state);

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
    void printStatusInternal(); // FIXED: Added declaration for the .cpp helper

    // MEMBER VARIABLES
    // Reordered to match Constructor Initialization: 
    // state_ -> doorOpen_ -> alarmManager_ -> logger_
    State state_ = State::ArmedIdle;
    bool doorOpen_ = false;
    AlarmManager& alarmManager_;
    AsyncLogger& logger_;

    mutable std::mutex mutex_;
    Ms authorizationWindow_{Ms(5000)};
    std::optional<Clock::time_point> verificationDeadline_;
};

#endif