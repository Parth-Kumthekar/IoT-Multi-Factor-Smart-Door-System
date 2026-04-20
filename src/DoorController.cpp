#include "DoorController.hpp"
#include "DoorAlarmSystem.h" // We need this to post events
#include <iostream>

/**
 * @brief Construct a new Door Controller.
 * @details Establishes the connection between the physical reed switch and the central orchestrator.
 * @param reedPin The GPIO offset for the magnetic sensor.
 * @param system Reference to the DoorAlarmSystem for event reporting.
 * @param oc Reference to the OutputController for signaling.
 */
DoorController::DoorController(int reedPin, DoorAlarmSystem& system, OutputController& oc)
    : reedSwitch(reedPin, false), mainSystem(system), outputController(oc) {}

/**
 * @brief Configures the reed switch and begins hardware monitoring.
 * @details Registers a lambda callback that translates a physical pin interrupt 
 * into a system-wide DoorOpened event. 
 * @note Uses a 50ms hardware debounce window.
 */
void DoorController::initialize() {
    // The callback now just notifies the System, it doesn't do logic
    reedSwitch.registerCallback([this]() { 
        // We report the physical event to the System
        mainSystem.postEvent(EventType::DoorOpened, "PhysicalReedSwitch");
    }, 50);
    
    reedSwitch.start();
}

/**
 * @brief Safely shuts down the hardware monitoring.
 */
void DoorController::stop() {
    reedSwitch.stop();
}

// REMOVED: onDoorOpen logic that was manually reading NFC.
// Reason: The NFC loop in DoorAlarmSystem is already constantly polling.
// When a card is tapped, the NFC loop posts an event, and the FSM 
// decides if that card was tapped within the 5-second window.
