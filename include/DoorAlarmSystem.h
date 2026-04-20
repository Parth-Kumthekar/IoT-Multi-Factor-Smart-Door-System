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
// Add these includes
#include "httplib.h" 
#include <mutex>

// ... existing includes ...

class DoorAlarmSystem {
public:
    DoorAlarmSystem();
    ~DoorAlarmSystem();

    void start();
    void stop();
    void postEvent(EventType type, const std::string& source);

private:
    void controlLoop(); 
    void timerLoop();   
    void apiLoop();     
    void nfcLoop(); 
    void onReedSwitchChange(int value);

    // Infrastructure
    std::atomic<bool> running_{false};
    std::mutex stateMtx_; 

    EventQueue eventQueue_;
    AsyncLogger logger_;       // Handles local CSV/Database saving internally
    AlarmManager alarmManager_; // Handles Siren + Email Alerts internally
    DoorAlarmFSM fsm_;

    // Hardware
    NFCReader nfcReader_;
    AccessController accessController_;
    OutputController outputController_;
    GPIOPin reedSwitch_;

    // Threads
    std::thread controlThread_;
    std::thread timerThread_;
    std::thread nfcThread_;
    std::thread apiThread_;
    
    httplib::Server svr_; 
};

#endif // DOORALARMSYSTEM_H