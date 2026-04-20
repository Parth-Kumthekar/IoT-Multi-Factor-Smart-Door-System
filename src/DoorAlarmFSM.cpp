#include "DoorAlarmFSM.h"
#include <sstream>

DoorAlarmFSM::DoorAlarmFSM(AlarmManager& alarmManager, AsyncLogger& logger)
    : state_(State::ArmedIdle),
      doorOpen_(false),
      alarmManager_(alarmManager),
      logger_(logger),
      authorizationWindow_(std::chrono::milliseconds(5000))
{
}

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
        logger_.log("FSM: Received unhandled event type.");
        break;
    }
}

void DoorAlarmFSM::handleDoorOpened(const std::string& source)
{
    doorOpen_ = true;
    logger_.log("FSM: Door OPENED (Source: " + source + ")");

    if (state_ == State::ArmedIdle)
    {
        state_ = State::PendingVerification;
        verificationDeadline_ = Clock::now() + authorizationWindow_;
        logger_.log("FSM: [AUDIT] Unauthorized Open. 5s grace period started.");
    }
}

void DoorAlarmFSM::handleDoorClosed(const std::string& source) 
{
    doorOpen_ = false;
    logger_.log("FSM: Door CLOSED (Source: " + source + ")");
    
    // Auto-Rearm logic
    if (state_ == State::AuthorizedEntry || state_ == State::PendingVerification || state_ == State::AlarmActive) {
        state_ = State::ArmedIdle;
        clearAuthorizationWindow();
        alarmManager_.clearAlarm(); 
        logger_.log("FSM: System re-secured. State -> ArmedIdle.");
    }
}

void DoorAlarmFSM::handleAuthorization(const Event& event) 
{
    std::string method = (event.type == EventType::AuthorizedByNfc) ? "NFC" : "APP";

    if (doorOpen_) {
        logger_.log("FSM: Auth via " + method + " REJECTED. Door is already open.");
        return; 
    }

    if (state_ == State::ArmedIdle || state_ == State::PendingVerification) 
    {
        state_ = State::AuthorizedEntry;
        clearAuthorizationWindow();
        alarmManager_.clearAlarm(); 
        logger_.log("FSM: [ACCESS GRANTED] via " + method + " (ID: " + event.source + "). Unlocking.");
    }
    else if (state_ == State::AlarmActive) {
        state_ = State::AuthorizedEntry;
        alarmManager_.clearAlarm();
        logger_.log("FSM: Alarm silenced by authorized " + method);
    }
}

void DoorAlarmFSM::handleVerificationTimeout(const std::string& source)
{
    if (state_ == State::PendingVerification)
    {
        state_ = State::AlarmActive;
        clearAuthorizationWindow();
        
        // Use the source parameter to satisfy the compiler and provide detail
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

// --- Getters & Status ---

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

DoorAlarmFSM::State DoorAlarmFSM::getState() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return state_;
}

std::string DoorAlarmFSM::getStateString() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return toString(state_);
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

void DoorAlarmFSM::setAuthorizationWindow(std::chrono::milliseconds window) {
    std::lock_guard<std::mutex> lock(mutex_);
    authorizationWindow_ = window;
}

void DoorAlarmFSM::clearAuthorizationWindow() {
    verificationDeadline_.reset();
}

std::string DoorAlarmFSM::toString(State state) {
    switch (state) {
        case State::Disarmed:           return "Disarmed";
        case State::ArmedIdle:          return "ArmedIdle";
        case State::PendingVerification: return "PendingVerification";
        case State::AuthorizedEntry:    return "AuthorizedEntry";
        case State::AlarmActive:        return "AlarmActive";
        case State::Fault:              return "Fault"; // Added to match header
        default:                        return "Unknown";
    }
}