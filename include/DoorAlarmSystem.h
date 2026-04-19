#ifndef DOORALARMSYSTEM_H
#define DOORALARMSYSTEM_H

#include "AlarmManager.h"
#include "AsyncLogger.h"
#include "DoorAlarmFSM.h"
#include "EventQueue.h"

#include <atomic>
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
    using Clock = std::chrono::steady_clock;
    using Ms = std::chrono::milliseconds;

    void controlLoop();
    void timerLoop();

private:
    std::atomic<bool> running_{false};

    EventQueue eventQueue_;
    AsyncLogger logger_;
    AlarmManager alarmManager_;
    DoorAlarmFSM fsm_;

    std::thread controlThread_;
    std::thread timerThread_;
};

#endif