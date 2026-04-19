#include "DoorAlarmFSM.h"
#include <sstream>
#include <algorithm>

// Constructor: Initializer list order MUST match the header file exactly
DoorAlarmFSM::DoorAlarmFSM(AlarmManager& alarmManager, AsyncLogger& logger)
    : state_(State::ArmedIdle),
      doorOpen_(false),
      alarmManager_(alarmManager),
      logger_(logger)
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
    }
}

void DoorAlarmFSM::handleDoorOpened(const std::string& source)
{
    doorOpen_ = true;
    logger_.log("FSM: Door OPENED (Source: " + source + ")");

    // If door opens without prior NFC/App authorization
    if (state_ == State::ArmedIdle)
    {
        state_ = State::PendingVerification;
        // Start the 5-second countdown (Timer loop in System class checks this)
        verificationDeadline_ = Clock::now() + authorizationWindow_;
        logger_.log("FSM: WARNING! Unauthorized Open. 5s window started.");
    }
}

void DoorAlarmFSM::handleDoorClosed(const std::string& source) 
{
    doorOpen_ = false;
    logger_.log("FSM: Door CLOSED (Source: " + source + ")");
    
    // Auto-Rearm Logic: Reset system to ArmedIdle when door shuts
    if (state_ == State::AuthorizedEntry || state_ == State::PendingVerification || state_ == State::AlarmActive) {
        state_ = State::ArmedIdle;
        clearAuthorizationWindow();
        alarmManager_.clearAlarm(); // Turn off buzzer immediately when door is closed
        logger_.log("FSM: Door secured. State -> ArmedIdle. System re-locked.");
    }
}

void DoorAlarmFSM::handleAuthorization(const Event& event) 
{
    std::string method = (event.type == EventType::AuthorizedByNfc) ? "NFC" : "APP";

    // RULE: Only check/allow UID if the door is physically CLOSED
    if (doorOpen_) {
        logger_.log("FSM: Auth via " + method + " REJECTED. Door must be CLOSED to scan.");
        return; 
    }

    // If door is closed, proceed with authorization
    if (state_ == State::ArmedIdle || state_ == State::PendingVerification) 
    {
        state_ = State::AuthorizedEntry;
        clearAuthorizationWindow();
        alarmManager_.clearAlarm(); // Stop buzzer if user scanned card within the 5s window
        
        logger_.log("FSM: ACCESS GRANTED via " + method + " (ID: " + event.source + "). Unlock active.");
    }
    else if (state_ == State::AlarmActive) {
        // Allow authorized NFC to silence an existing alarm
        state_ = State::AuthorizedEntry;
        alarmManager_.clearAlarm();
        logger_.log("FSM: Alarm silenced by authorized " + method);
    }
}

void DoorAlarmFSM::handleVerificationTimeout(const std::string& source)
{
    // Triggered by the background timer thread after 5 seconds
    if (state_ == State::PendingVerification)
    {
        state_ = State::AlarmActive;
        clearAuthorizationWindow();
        
        // This activates the Buzzer (via AlarmManager)
        alarmManager_.triggerAlarm("Unauthorized entry timeout: Source " + source);
        logger_.log("FSM: TIMEOUT REACHED! Buzzer ON. State -> AlarmActive.");
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

// --- Status Helpers ---

void DoorAlarmFSM::printStatus()
{
    std::lock_guard<std::mutex> lock(mutex_);
    printStatusInternal();
}

void DoorAlarmFSM::printStatusInternal()
{
    std::ostringstream oss;
    oss << "[DEBUG STATUS] "
        << "FSM: " << toString(state_) << " | "
        << "DOOR: " << (doorOpen_ ? "OPEN (1)" : "CLOSED (0)") << " | "
        << "ALARM: " << (alarmManager_.isAlarmActive() ? "ON" : "OFF");
    logger_.log(oss.str());
}

// --- Getters and Boilerplate ---

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

std::optional<DoorAlarmFSM::Clock::time_point> DoorAlarmFSM::getVerificationDeadline() const
{
    std::lock_guard<std::mutex> lock(mutex_);
    return verificationDeadline_;
}

void DoorAlarmFSM::clearAuthorizationWindow()
{
    verificationDeadline_.reset();
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
    default:                        return "Unknown";
    }
}