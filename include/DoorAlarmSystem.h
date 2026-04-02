#ifndef DOORALARMSYSTEM_H
#define DOORALARMSYSTEM_H

#include "AlarmManager.h"
#include "AsyncLogger.h"
#include "EventQueue.h"

#include <atomic>
#include <chrono>
#include <mutex>
#include <optional>
#include <string>
#include <thread>

class DoorAlarmSystem
{
public:
    DoorAlarmSystem();
    ~DoorAlarmSystem();

    void start();
    void stop();
    void postEvent(EventType type, const std::string& source);

private:
    enum class SystemState
    {
        Disarmed,
        ArmedIdle,
        PendingVerification,
        AuthorizedEntry,
        AlarmActive,
        Fault
    };

    void controlLoop();
    void timerLoop();
    void handleEvent(const Event& event);

    void handleArm(const std::string& source);
    void handleDisarm(const std::string& source);
    void handleDoorOpened(const std::string& source);
    void handleDoorClosed(const std::string& source);
    void handleAuthorization(const Event& event);
    void handleVerificationTimeout(const std::string& source);

    void printStatus();
    void clearAuthorizationWindow();

    static std::string toString(SystemState state);

private:
    using Clock = std::chrono::steady_clock;
    using Ms = std::chrono::milliseconds;

    std::atomic<bool> running_{false};

    EventQueue eventQueue_;
    AsyncLogger logger_;
    AlarmManager alarmManager_;

    std::thread controlThread_;
    std::thread timerThread_;

    std::mutex stateMutex_;
    SystemState state_ = SystemState::Disarmed;
    bool doorOpen_ = false;

    Ms authorizationWindow_{Ms(5000)};
    std::optional<Clock::time_point> verificationDeadline_;
};

#endif