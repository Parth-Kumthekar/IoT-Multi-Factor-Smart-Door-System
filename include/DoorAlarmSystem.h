#ifndef DOORALARMSYSTEM_H
#define DOORALARMSYSTEM_H

#include "AlarmManager.h"
#include "AsyncLogger.h"
#include "DoorAlarmFSM.h"
#include "EventQueue.h"

// Hardware and Logic Includes from other branches
#include "NFCReader.hpp"
#include "AccessController.hpp"
#include "OutputController.hpp"
#include "gpiopin.hpp"

#include <atomic>
#include <thread>
#include <string>

class DoorAlarmSystem
{
public:
    DoorAlarmSystem();
    ~DoorAlarmSystem();

    void start();
    void stop();
    
    // Allows any thread to push events (Door open, NFC scan, etc.)
    void postEvent(EventType type, const std::string& source);

private:
    using Clock = std::chrono::steady_clock;
    using Ms = std::chrono::milliseconds;

    // --- Core Logic Loops ---
    void controlLoop(); // Processes the event queue
    void timerLoop();   // Handles the 5-second verification timeout

    // --- Hardware Monitoring Loops ---
    void nfcLoop();     // Constantly polls UART for NFC tags
    void onReedSwitchChange(int value); // Callback for the door sensor

private:
    std::atomic<bool> running_{false};

    // Infrastructure
    EventQueue eventQueue_;
    AsyncLogger logger_;
    AlarmManager alarmManager_;
    DoorAlarmFSM fsm_;

    // Hardware Interface Objects
    NFCReader nfcReader_;
    AccessController accessController_;
    OutputController outputController_;
    GPIOPin reedSwitch_;

    // Threads
    std::thread controlThread_;
    std::thread timerThread_;
    std::thread nfcThread_;
};

#endif