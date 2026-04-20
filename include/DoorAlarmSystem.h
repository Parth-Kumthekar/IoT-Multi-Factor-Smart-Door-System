#ifndef DOORALARMSYSTEM_H
#define DOORALARMSYSTEM_H

#include "AlarmManager.h"
#include "AsyncLogger.h"
#include "DoorAlarmFSM.h"
#include "EventQueue.h"
#include "NFCReader.hpp"
#include "AccessController.hpp"
#include "OutputController.hpp"
#include "gpiopin.hpp"
#include "httplib.h" 

#include <atomic>
#include <thread>
#include <string>
#include <mutex>

/**
 * @class DoorAlarmSystem
 * @brief The primary orchestrator for the IoT Door Lock system.
 * * This class manages the lifecycle of all hardware interfaces, background 
 * threads, and the central logic engine. It implements a multi-threaded 
 * architecture to handle concurrent tasks such as NFC scanning, HTTP API 
 * requests, and real-time state transitions without blocking.
 */
class DoorAlarmSystem {
public:
    /**
     * @brief Initializes all hardware components and prepares internal state.
     */
    DoorAlarmSystem();

    /**
     * @brief Cleans up hardware connections and ensures all threads are joined safely.
     */
    ~DoorAlarmSystem();

    /**
     * @brief Starts all operational threads (Control, Timer, NFC, and Web API).
     */
    void start();

    /**
     * @brief Signals all threads to stop and transitions the system to a safe state.
     */
    void stop();

    /**
     * @brief Thread-safe method to inject events into the system's main processing queue.
     * @param type The category of the event (e.g., NFC_SCANNED, SENSOR_TRIGGERED).
     * @param source Metadata string describing the origin or detail of the event.
     */
    void postEvent(EventType type, const std::string& source);

private:
    /** @brief Main logic thread: Consumes events from the queue and updates the FSM. */
    void controlLoop(); 

    /** @brief Temporal thread: Handles timeouts and periodic system health checks. */
    void timerLoop();   

    /** @brief Web Interface thread: Manages the REST API for remote status monitoring. */
    void apiLoop();     

    /** @brief Dedicated hardware thread: Continuously polls the NFC reader for credentials. */
    void nfcLoop(); 

    /**
     * @brief Interrupt-driven callback handler for the physical Reed switch (Door sensor).
     * @param value The current state of the GPIO pin.
     */
    void onReedSwitchChange(int value);

    // Infrastructure
    /** @brief Atomic control flag for safe multi-threaded shutdown. */
    std::atomic<bool> running_{false};
    
    /** @brief Mutex to protect shared resources within the system orchestrator. */
    std::mutex stateMtx_; 

    /** @brief Thread-safe event queue (Producer-Consumer pattern). */
    EventQueue eventQueue_;

    /** @brief Background logging service for non-blocking I/O. */
    AsyncLogger logger_;

    /** @brief Hardware manager for siren and feedback alerts. */
    AlarmManager alarmManager_;

    /** @brief Central Finite State Machine for security logic. */
    DoorAlarmFSM fsm_;

    // Hardware Interfaces
    NFCReader nfcReader_;
    AccessController accessController_;
    OutputController outputController_;
    GPIOPin reedSwitch_;

    // Thread Management
    std::thread controlThread_;
    std::thread timerThread_;
    std::thread nfcThread_;
    std::thread apiThread_;
    
    /** @brief Embedded HTTP server for status reporting and remote control. */
    httplib::Server svr_; 
};

#endif // DOORALARMSYSTEM_H