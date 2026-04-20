#include "DoorAlarmFSM.h"
#include <sstream>

/**
 * @brief Constructor for the FSM logic engine.
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
 * @brief Central dispatcher for all system events.
 * @details Thread-safe transition logic using a std::lock_guard.
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
    default:
        // Log unhandled events for debugging
        break;
    }
}

// --- PUBLIC ACCESSORS (Fixed Linker Errors) ---

DoorAlarmFSM::State DoorAlarmFSM::getState() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return state_;
}

bool DoorAlarmFSM::isDoorOpen() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return doorOpen_;
}

bool DoorAlarmFSM::isAlarmActive() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return state_ == State::AlarmActive;
}

std::optional<DoorAlarmFSM::Clock::time_point> DoorAlarmFSM::getVerificationDeadline() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return verificationDeadline_;
}

// --- EVENT HANDLERS ---

void DoorAlarmFSM::handleArm(const std::string& source) {
    state_ = State::ArmedIdle;
    logger_.log("SYSTEM", "System ARMED via " + source);
}

void DoorAlarmFSM::handleDisarm(const std::string& source) {
    state_ = State::Disarmed;
    clearAuthorizationWindow();
    alarmManager_.clearAlarm();
    logger_.log("SYSTEM", "System DISARMED via " + source);
}

void DoorAlarmFSM::handleDoorOpened(const std::string& source) {
    doorOpen_ = true;
    logger_.log("HARDWARE", "Door OPENED (Source: " + source + ")");

    if (state_ == State::ArmedIdle) {
        state_ = State::PendingVerification;
        verificationDeadline_ = Clock::now() + authorizationWindow_;
        logger_.log("SECURITY", "Unauthorized Open. Grace period started.");
    }
}

void DoorAlarmFSM::handleDoorClosed(const std::string& source) {
    doorOpen_ = false;
    logger_.log("HARDWARE", "Door CLOSED (Source: " + source + ")");
    
    // Automatic re-arm if door closes during entry or alarm
    if (state_ != State::Disarmed) {
        state_ = State::ArmedIdle;
        clearAuthorizationWindow();
        alarmManager_.clearAlarm(); 
        logger_.log("SECURITY", "System re-secured. State -> ArmedIdle.");
    }
}

void DoorAlarmFSM::handleAuthorization(const Event& event) {
    std::string method = (event.type == EventType::AuthorizedByNfc) ? "NFC" : "APP";

    if (doorOpen_) {
        logger_.log("ACCESS", "Auth via " + method + " REJECTED. Door is already open.");
        return; 
    }

    if (state_ == State::ArmedIdle || state_ == State::PendingVerification || state_ == State::AlarmActive) {
        state_ = State::AuthorizedEntry;
        clearAuthorizationWindow();
        alarmManager_.clearAlarm(); 
        logger_.log("ACCESS", "GRANTED via " + method + " (ID: " + event.source + "). Unlocking.");
    }
}

void DoorAlarmFSM::handleVerificationTimeout(const std::string& source) {
    if (state_ == State::PendingVerification) {
        state_ = State::AlarmActive;
        clearAuthorizationWindow();
        alarmManager_.triggerAlarm("Timeout triggered by: " + source);
        logger_.log("SECURITY", "ALARM: Grace period expired!");
    }
}

// --- HELPERS & UTILITIES ---

void DoorAlarmFSM::setAuthorizationWindow(Ms window) {
    std::lock_guard<std::mutex> lock(mutex_);
    authorizationWindow_ = window;
}

void DoorAlarmFSM::clearAuthorizationWindow() {
    verificationDeadline_.reset();
}

void DoorAlarmFSM::printStatus() {
    std::lock_guard<std::mutex> lock(mutex_);
    printStatusInternal();
}

void DoorAlarmFSM::printStatusInternal() {
    logger_.log("STATUS", "State: " + toString(state_) + " | Door: " + (doorOpen_ ? "Open" : "Closed"));
}

/**
 * @brief Static helper to convert State enum to string.
 * FIX: This satisfies the Linker error for DoorAlarmSystem.cpp
 */
std::string DoorAlarmFSM::toString(State state) {
    switch (state) {
        case State::Disarmed:           return "Disarmed";
        case State::ArmedIdle:          return "ArmedIdle";
        case State::PendingVerification: return "PendingVerification";
        case State::AuthorizedEntry:    return "AuthorizedEntry";
        case State::AlarmActive:        return "AlarmActive";
        case State::Fault:              return "Fault";
        default:                        return "Unknown";
    }
}