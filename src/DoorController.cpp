#include "DoorController.hpp"
#include "DoorAlarmSystem.h" // We need this to post events
#include <iostream>

// Pass a pointer or reference to the main system to "Post" events
DoorController::DoorController(int reedPin, DoorAlarmSystem& system, OutputController& oc)
    : reedSwitch(reedPin, false), mainSystem(system), outputController(oc) {}

void DoorController::initialize() {
    // The callback now just notifies the System, it doesn't do logic
    reedSwitch.registerCallback([this]() { 
        // We report the physical event to the System
        mainSystem.postEvent(EventType::DoorOpened, "PhysicalReedSwitch");
    }, 50);
    
    reedSwitch.start();
}

void DoorController::stop() {
    reedSwitch.stop();
}

// REMOVED: onDoorOpen logic that was manually reading NFC.
// Reason: The NFC loop in DoorAlarmSystem is already constantly polling.
// When a card is tapped, the NFC loop posts an event, and the FSM 
// decides if that card was tapped within the 5-second window.