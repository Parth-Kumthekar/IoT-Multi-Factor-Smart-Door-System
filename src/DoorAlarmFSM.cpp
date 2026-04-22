/**
 * @file DoorAlarmFSM.cpp
 * @brief Implementation of the Finite State Machine for the Door Alarm System.
 */

#include "DoorAlarmFSM.h"
#include <sstream>

/**
 * @brief Construct a new Door Alarm FSM object.
 * @param alarmManager Reference to the system's alarm control module.
 * @param logger Reference to the asynchronous logging service.
 */
DoorAlarmFSM::DoorAlarmFSM(AlarmManager& alarmManager, AsyncLogger& logger)
    : state_(State::ArmedIdle),
      doorOpen_(false),
      alarmManager_(alarmManager),
      logger_(logger),
      authorizationWindow_(std::chrono::milliseconds(5000))
{
}

/**
 * @brief Primary entry point for all system events.
 * @details Thread-safely dispatches events to specific internal handlers.
 * @param event The event structure containing type and source metadata.
 */
void DoorAlarmFSM::handleEvent(const Event& event)
{
    std::lock_guard<std::mutex> lock(mutex_);

    switch (event.type)
    {
    case EventType::ArmSystem:
        handleArm(event.source);
        break;
    case EventType::DisarmSystem:
        handleDisarm(event.source);
        break;
    case EventType::DoorOpened:
        handleDoorOpened(event.source);
        break;
    case EventType::DoorClosed:
        handleDoorClosed(event.source);
        break;
    case EventType::AuthorizedByNfc:
    case EventType::AuthorizedByApp:
        handleAuthorization(event);
        break;
    case EventType::VerificationTimeout:
        handleVerificationTimeout(event.source);
        break;
    case EventType::PrintStatus:
        printStatusInternal(); 
        break;
    case EventType::Shutdown:
        break;
    default:
        logger_.log("FSM: Received unhandled event type: " + std::to_string(static_cast<int>(event.type)));
        break;
    }
}

/**
 * @brief Processes physical door opening signals.
 */
void DoorAlarmFSM::handleDoorOpened(const std::string& source)
{
    doorOpen_ = true;
    logger_.log("FSM: Door OPENED (Source: " + source + ")");

    // If we are in ArmedIdle, someone opened the door without authorization.
    if (state_ == State::ArmedIdle)
    {
        state_ = State::PendingVerification;
        verificationDeadline_ = Clock::now() + authorizationWindow_;
        logger_.log("FSM: [AUDIT] Unauthorized Open. 5s grace period started.");
    }
    // If we are in AuthorizedEntry, the door was opened legally. We stay in this state
    // until the timer or door closing event moves us back to ArmedIdle.
}

/**
 * @brief Processes physical door closing signals.
 */
void DoorAlarmFSM::handleDoorClosed(const std::string& source) 
{
    doorOpen_ = false;
    logger_.log("FSM: Door CLOSED (Source: " + source + ")");
    
    // Auto-Rearm: Once the door is shut, we ensure the system is secured.
    if (state_ == State::AuthorizedEntry || state_ == State::PendingVerification || state_ == State::AlarmActive) {
        state_ = State::ArmedIdle;
        clearAuthorizationWindow();
        alarmManager_.clearAlarm(); 
        logger_.log("FSM: System re-secured. State -> ArmedIdle.");
    }
}

/**
 * @brief Validates and processes access attempts (NFC or App).
 * @details Gatekeeper: Rejects authorization if the door is already physically open.
 */
void DoorAlarmFSM::handleAuthorization(const Event& event) 
{
    std::string method = (event.type == EventType::AuthorizedByNfc) ? "NFC" : "APP";

    // SECURITY GATEKEEPER
    if (doorOpen_) {
        logger_.log("FSM: [SECURITY REJECTION] Auth via " + method + " ignored. Door must be CLOSED to authorize via NFC/APP.");
        return; 
    }

    if (state_ == State::ArmedIdle || state_ == State::PendingVerification || state_ == State::AlarmActive) 
    {
        state_ = State::AuthorizedEntry;
        clearAuthorizationWindow();
        alarmManager_.clearAlarm(); 
        logger_.log("FSM: [ACCESS GRANTED] via " + method + " (ID: " + event.source + ").");
    }
}

/**
 * @brief Transitions to AlarmActive if grace period expires.
 */
void DoorAlarmFSM::handleVerificationTimeout(const std::string& source)
{
    if (state_ == State::PendingVerification)
    {
        state_ = State::AlarmActive;
        clearAuthorizationWindow();
        alarmManager_.triggerAlarm("Timeout triggered by: " + source);
        logger_.log("FSM: [ALARM] Grace period expired! Source: " + source);
    }
}

void DoorAlarmFSM::handleArm(const std::string& source)
{
    if (state_ == State::Disarmed || state_ == State::AuthorizedEntry)
    {
        state_ = State::ArmedIdle;
        clearAuthorizationWindow();
        alarmManager_.clearAlarm();
        logger_.log("FSM: System ARMED by " + source);
    }
}

void DoorAlarmFSM::handleDisarm(const std::string& source)
{
    state_ = State::Disarmed;
    clearAuthorizationWindow();
    alarmManager_.clearAlarm();
    logger_.log("FSM: System DISARMED by " + source);
}

// --- Thread-Safe Getters & Setters ---

void DoorAlarmFSM::setDoorState(bool isOpen) {
    std::lock_guard<std::mutex> lock(mutex_);
    doorOpen_ = isOpen;
}

bool DoorAlarmFSM::isDoorOpen() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return doorOpen_;
}

DoorAlarmFSM::State DoorAlarmFSM::getState() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return state_;
}

bool DoorAlarmFSM::isAlarmActive() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return state_ == State::AlarmActive;
}

std::optional<DoorAlarmFSM::Clock::time_point> DoorAlarmFSM::getVerificationDeadline() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return verificationDeadline_;
}

void DoorAlarmFSM::setAuthorizationWindow(std::chrono::milliseconds window) {
    std::lock_guard<std::mutex> lock(mutex_);
    authorizationWindow_ = window;
}

// --- Internal Helpers ---

void DoorAlarmFSM::clearAuthorizationWindow() {
    verificationDeadline_.reset();
}

void DoorAlarmFSM::printStatus() {
    std::lock_guard<std::mutex> lock(mutex_);
    printStatusInternal();
}

void DoorAlarmFSM::printStatusInternal() {
    std::ostringstream oss;
    oss << "[STATUS] State: " << toString(state_) 
        << " | Door: " << (doorOpen_ ? "OPEN" : "CLOSED")
        << " | Alarm: " << (state_ == State::AlarmActive ? "ACTIVE" : "OFF");
    logger_.log(oss.str());
}

std::string DoorAlarmFSM::toString(State state) {
    switch (state) {
        case State::Disarmed:            return "Disarmed";
        case State::ArmedIdle:           return "ArmedIdle";
        case State::PendingVerification: return "PendingVerification";
        case State::AuthorizedEntry:     return "AuthorizedEntry";
        case State::AlarmActive:         return "AlarmActive";
        case State::Fault:               return "Fault";
        default:                         return "Unknown";
    }
}
