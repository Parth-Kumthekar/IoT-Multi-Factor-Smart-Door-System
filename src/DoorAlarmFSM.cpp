#include "DoorAlarmFSM.h"
#include <sstream>
#include <algorithm>

// Constructor: Set initial state to ArmedIdle so it's ready immediately
DoorAlarmFSM::DoorAlarmFSM(AlarmManager& alarmManager, AsyncLogger& logger)
    : state_(State::ArmedIdle), doorOpen_(false), alarmManager_(alarmManager), logger_(logger)
{
}

void DoorAlarmFSM::setAuthorizationWindow(Ms window)
{
    std::lock_guard<std::mutex> lock(mutex_);
    authorizationWindow_ = window;
}

DoorAlarmFSM::State DoorAlarmFSM::getState() const
{
    std::lock_guard<std::mutex> lock(mutex_);
    return state_;
}

bool DoorAlarmFSM::isDoorOpen() const
{
    std::lock_guard<std::mutex> lock(mutex_);
    return doorOpen_;
}

bool DoorAlarmFSM::isAlarmActive() const
{
    return alarmManager_.isAlarmActive();
}

std::optional<DoorAlarmFSM::Clock::time_point> DoorAlarmFSM::getVerificationDeadline() const
{
    std::lock_guard<std::mutex> lock(mutex_);
    return verificationDeadline_;
}

std::string DoorAlarmFSM::toString(State state)
{
    switch (state)
    {
    case State::Disarmed:           return "Disarmed";
    case State::ArmedIdle:          return "ArmedIdle";
    case State::PendingVerification: return "PendingVerification";
    case State::AuthorizedEntry:    return "AuthorizedEntry";
    case State::AlarmActive:        return "AlarmActive";
    case State::Fault:              return "Fault";
    default:                        return "Unknown";
    }
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
        printStatus();
        break;
    case EventType::Shutdown:
        break;
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

void DoorAlarmFSM::handleDoorOpened(const std::string& source)
{
    doorOpen_ = true;
    logger_.log("FSM: Door OPENED (Source: " + source + ")");

    // If door opens while Armed and no Authorization has occurred yet
    if (state_ == State::ArmedIdle)
    {
        state_ = State::PendingVerification;
        verificationDeadline_ = Clock::now() + authorizationWindow_;
        logger_.log("FSM: State -> PendingVerification. Waiting for authorization.");
    }
}

void DoorAlarmFSM::handleDoorClosed(const std::string& source) 
{
    doorOpen_ = false;
    logger_.log("FSM: Door CLOSED (Source: " + source + ")");
    
    // Logic: Once the door is physically shut, if we were in an Authorized state,
    // we re-arm the system and lock the door.
    if (state_ == State::AuthorizedEntry) {
        state_ = State::ArmedIdle;
        logger_.log("FSM: Door secured. State -> ArmedIdle. Lock Re-engaged.");
    }
}

void DoorAlarmFSM::handleAuthorization(const Event& event) 
{
    std::string method = (event.type == EventType::AuthorizedByNfc) ? "NFC" : "CAMERA";

    // Accept authorization if Armed, Disarmed, or currently in the verification grace period
    if (state_ == State::ArmedIdle || state_ == State::Disarmed || state_ == State::PendingVerification) 
    {
        state_ = State::AuthorizedEntry;
        clearAuthorizationWindow();
        alarmManager_.clearAlarm();
        
        logger_.log("FSM: ACCESS GRANTED via " + method + " (ID: " + event.source + ")");
        return; 
    }

    if (state_ == State::AlarmActive) {
        logger_.log("FSM: Auth via " + method + " rejected. Alarm is ACTIVE! Reset required.");
        return;
    }

    logger_.log("FSM: Auth ignored in state " + toString(state_));
}

void DoorAlarmFSM::handleVerificationTimeout(const std::string& source)
{
    if (state_ == State::PendingVerification)
    {
        state_ = State::AlarmActive;
        clearAuthorizationWindow();
        alarmManager_.triggerAlarm("Unauthorized entry timeout: No verification received");
        logger_.log("FSM: TIMEOUT! State -> AlarmActive.");
    }
}

void DoorAlarmFSM::printStatus()
{
    std::ostringstream oss;
    oss << "FSM STATUS: [State: " << toString(state_)
        << "] [Door: " << (doorOpen_ ? "OPEN" : "CLOSED")
        << "] [Alarm: " << (alarmManager_.isAlarmActive() ? "ON" : "OFF") << "]";
    logger_.log(oss.str());
}

void DoorAlarmFSM::clearAuthorizationWindow()
{
    verificationDeadline_.reset();
}

void DoorAlarmFSM::printStatus()
{
    std::lock_guard<std::mutex> lock(mutex_);
    std::ostringstream oss;
    
    // Debug info: State + Door + Alarm
    oss << "[DEBUG] "
        << "FSM: " << toString(state_) << " | "
        << "DOOR: " << (doorOpen_ ? "OPEN (1)" : "CLOSED (0)") << " | "
        << "ALARM: " << (alarmManager_.isAlarmActive() ? "ACTIVE" : "OFF");

    logger_.log(oss.str());
}