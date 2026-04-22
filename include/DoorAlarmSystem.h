#ifndef DOORALARMSYSTEM_H
#define DOORALARMSYSTEM_H

#include "AlarmManager.h"
#include "AsyncLogger.h"
#include "DoorAlarmFSM.h"
#include "EventQueue.h"

// Hardware and Logic Includes
#include "NFCReader.hpp"
#include "AccessController.hpp"
#include "OutputController.hpp"
#include "gpiopin.hpp"

#include <atomic>
#include <thread>
#include <string>
#include <mutex>
#include "httplib.h" 

/**
 * @class DoorAlarmSystem
 * @brief The primary orchestrator for the door security project.
 * @details Encapsulates hardware components, the state machine, and 
 * infrastructure services. It manages multiple threads for hardware polling,
 * core logic, and the REST API.
 */
class DoorAlarmSystem {
public:
    /**
     * @brief Construct a new Door Alarm System object.
     */
    DoorAlarmSystem();

    /**
     * @brief Destroy the Door Alarm System object.
     * @note Ensures all worker threads are joined and hardware pins are safely released.
     */
    ~DoorAlarmSystem();

    /**
     * @brief Spawns all background threads and begins system operation.
     */
    void start();

    /**
     * @brief Signals all threads to stop and performs a clean shutdown.
     */
    void stop();

    /**
     * @brief Manually injects an event into the system's processing queue.
     * @param type The category of the event (Door, NFC, System).
     * @param source A string identifying the origin of the event.
     */
    void postEvent(EventType type, const std::string& source);

private:
    // --- Background Service Threads ---
    void controlLoop();  ///< Core logic thread: consumes events and updates FSM.
    void timerLoop();    ///< Monitors time-sensitive logic and auto-resets.
    void apiLoop();      ///< HTTP server for remote monitoring.
    void nfcLoop();      ///< Polling thread for NFC hardware.

    // --- Hardware Callbacks ---
    void onReedSwitchChange(int value);
    void onButtonPress(int value);

    // --- Logic Helpers ---
    /**
     * @brief Centralized OR-gate logic for the Green LED.
     * @details (FSM Authorized AND Door Closed) OR (External Camera Active).
     */
    void updateGreenLedLogic();

    // --- Infrastructure ---
    std::atomic<bool> running_{false};
    std::mutex stateMtx_; 
    EventQueue eventQueue_;
    AsyncLogger logger_;       
    AlarmManager alarmManager_; 
    DoorAlarmFSM fsm_;

    // --- Hardware Interfaces ---
    NFCReader nfcReader_;
    AccessController accessController_;
    OutputController outputController_;
    
    GPIOPin reedSwitch_;   ///< Magnetic door sensor.
    GPIOPin exitButton_;   ///< Internal push button for exit.
    GPIOPin cameraTrigger_; ///< External input from Camera Block (Pin 17).

    // --- State Tracking ---
    bool cameraActive_;    ///< Tracks the physical state of the Camera Trigger pin.

    // --- Thread Management ---
    std::thread controlThread_;
    std::thread timerThread_; 
    std::thread nfcThread_;   
    std::thread apiThread_;   
    
    httplib::Server svr_; 
};

#endif // DOORALARMSYSTEM_H
