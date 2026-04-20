#include "DoorController.hpp"
#include "DoorAlarmSystem.h" 
#include <iostream>

/**
 * @brief Constructor for the DoorController.
 * @param reedPin Physical BCM pin for the door sensor.
 * @param system Reference to the main orchestrator for event reporting.
 * @param oc Reference to the output hardware for local feedback.
 */
DoorController::DoorController(int reedPin, DoorAlarmSystem& system, OutputController& oc)
    : reedSwitch(reedPin, false), mainSystem(system), outputController(oc) {}

/**
 * @brief Binds the hardware interrupt to the system-wide event queue.
 * * This method implements the 'Callback' requirement. Instead of the 
 * hardware layer processing logic, it asynchronously 'posts' a message 
 * to the orchestrator, ensuring the ISR (Interrupt Service Routine) remains 
 * extremely fast and non-blocking.
 */
void DoorController::initialize() {
    
    // Register a lambda as the GPIO callback
    // The '50' represents a 50ms hardware debounce window to prevent false triggers
    reedSwitch.registerCallback([this]() { 
        
        // Strategy: Delegate all logic to the FSM via the EventQueue
        mainSystem.postEvent(EventType::DoorOpened, "PhysicalReedSwitch");
        
    }, 50);
    
    reedSwitch.start();
}

/**
 * @brief Safely shuts down the GPIO worker thread.
 */
void DoorController::stop() {
    reedSwitch.stop();
}