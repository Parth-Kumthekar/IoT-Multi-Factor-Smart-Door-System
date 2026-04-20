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

/**
 * @class DoorAlarmSystem
 * @brief The primary orchestrator for the door security project.
 * * This class encapsulates all hardware components, the state machine, and 
 * infrastructure services. It manages multiple execution threads for 
 * hardware polling (NFC, GPIO), the core logic loop, and the REST API server.
 */
class DoorAlarmSystem {
public:
    /**
     * @brief Construct a new Door Alarm System object.
     * @details Initializes hardware controllers, the FSM, and required buffers.
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
    /**
     * @brief The core logic thread. Consumes events from the queue and updates the FSM.
     */
    void controlLoop(); 

    /**
     * @brief Monitors time-sensitive logic, such as the verification window timeout.
     */
    void timerLoop();   

    /**
     * @brief Background thread running the HTTP server for remote monitoring/API access.
     */
    void apiLoop();     

    /**
     * @brief Dedicated thread for polling the NFC reader hardware.
     */
    void nfcLoop(); 

    /**
     * @brief Callback function triggered by GPIO interrupts when the door sensor changes.
     * @param value The current state of the pin (0 or 1).
     */
    void onReedSwitchChange(int value);

    // Infrastructure
    /// Global flag indicating if the system's background threads should remain active.
    std::atomic<bool> running_{false};
    
    /// Mutex for protecting shared system-wide configuration or state access.
    std::mutex stateMtx_; 

    /// Thread-safe queue for buffering events between hardware threads and the control loop.
    EventQueue eventQueue_;
    
    /// Service for non-blocking logging to disk/CSV.
    AsyncLogger logger_;       

    /// Logic for managing siren states and alerts.
    AlarmManager alarmManager_; 

    /// The core Finite State Machine logic.
    DoorAlarmFSM fsm_;

    // Hardware
    /// Interface for the physical NFC/RFID reader.
    NFCReader nfcReader_;
    
    /// Logic for validating scanned UIDs against a whitelist.
    AccessController accessController_;
    
    /// Controller for physical outputs (Siren, LEDs, etc.).
    OutputController outputController_;
    
    /// GPIO interface for the magnetic door reed switch.
    GPIOPin reedSwitch_;

    // Threads
    std::thread controlThread_; ///< Handle for the logic processing thread.
    std::thread timerThread_;   ///< Handle for the temporal monitoring thread.
    std::thread nfcThread_;     ///< Handle for the hardware polling thread.
    std::thread apiThread_;     ///< Handle for the web server thread.
    
    /// The REST API server instance provided by httplib.
    httplib::Server svr_; 
};

#endif // DOORALARMSYSTEM_H
