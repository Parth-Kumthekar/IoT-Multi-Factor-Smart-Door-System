#include "DoorAlarmFSM.h"
#include <sstream>
#include <algorithm>

/**
 * Constructor
 * Note: Initializer list order matches the header.
 */
DoorAlarmFSM::DoorAlarmFSM(AlarmManager& alarmManager, AsyncLogger& logger)
    : state_(State::ArmedIdle),
      doorOpen_(false),
      alarmManager_(alarmManager),
      logger_(logger),
      authorizationWindow_(std::chrono::milliseconds(5000)) // Default 5s
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
        // No action needed for FSM logic on shutdown
        break;
    }
}

void DoorAlarmFSM::handleDoorOpened(const std::string& source)
{
    doorOpen_ = true;
    logger_.log("FSM: Door OPENED (Source: " + source + ")");

    // If door opens while system is Armed and not already expecting someone
    if (state_ == State::ArmedIdle)
    {
        state_ = State::PendingVerification;
        // Start the countdown window
        verificationDeadline_ = Clock::now() + authorizationWindow_;
        logger_.log("FSM: [AUDIT] Unauthorized Open. 5s grace period started for verification.");
    }
}

void DoorAlarmFSM::handleDoorClosed(const std::string& source) 
{
    doorOpen_ = false;
    logger_.log("FSM: Door CLOSED (Source: " + source + ")");
    
    // Auto-Rearm Logic: Reset to ArmedIdle when door shuts
    if (state_ == State::AuthorizedEntry || state_ == State::PendingVerification || state_ == State::AlarmActive) {
        state_ = State::ArmedIdle;
        clearAuthorizationWindow();
        alarmManager_.clearAlarm(); // Stop buzzer immediately
        logger_.log("FSM: System re-secured. State -> ArmedIdle.");
    }
}

void DoorAlarmFSM::handleAuthorization(const Event& event) 
{
    std::string method = (event.type == EventType::AuthorizedByNfc) ? "NFC" : "APP";

    // Security Rule: Require door to be CLOSED for initial authorization scan
    // This prevents someone already inside from "authorizing" while the door is open.
    if (doorOpen_) {
        logger_.log("FSM: Auth via " + method + " REJECTED. Door is already open.");
        return; 
    }

    if (state_ == State::ArmedIdle || state_ == State::PendingVerification) 
    {
        state_ = State::AuthorizedEntry;
        clearAuthorizationWindow();
        alarmManager_.clearAlarm(); // Silence buzzer if user scanned during grace period
        
        logger_.log("FSM: [ACCESS GRANTED] via " + method + " (ID: " + event.source + "). Unlocking.");
    }
    else if (state_ == State::AlarmActive) {
        // Authorized scans can silence an existing alarm
        state_ = State::AuthorizedEntry;
        alarmManager_.clearAlarm();
        logger_.log("FSM: Alarm silenced/cleared by authorized " + method);
    }
}

void DoorAlarmFSM::handleVerificationTimeout(const std::string& source)
{
    // If we were waiting for a scan and the timer thread fires:
    if (state_ == State::PendingVerification)
    {
        state_ = State::AlarmActive;
        clearAuthorizationWindow();
        
        // This triggers both the Hardware Buzzer AND the Email Alert thread inside AlarmManager
        alarmManager_.triggerAlarm("CRITICAL: Unauthorized entry timeout. No authorization received.");
        
        logger_.log("FSM: [ALARM] Grace period expired! Triggering external alerts.");
    }
}

void DoorAlarmFSM::handleArm(const std::string& source)
{
    if (state_ == State::Disarmed || state_ == State::AuthorizedEntry)
    {
        state_ = State::ArmedIdle;
        clearAuthorizationWindow();
        alarmManager_.clearAlarm();
        logger_.log("FSM: System ARMED manually by " + source);
    }
}

void DoorAlarmFSM::handleDisarm(const std::string& source)
{
    state_ = State::Disarmed;
    clearAuthorizationWindow();
    alarmManager_.clearAlarm();
    logger_.log("FSM: System DISARMED manually by " + source);
}

// --- Status Helpers (Used by Web API / Logger) ---

void DoorAlarmFSM::printStatus()
{
    std::lock_guard<std::mutex> lock(mutex_);
    printStatusInternal();
}

void DoorAlarmFSM::printStatusInternal()
{
    std::ostringstream oss;
    oss << "[SYSTEM STATUS] "
        << "FSM: " << toString(state_) << " | "
        << "DOOR: " << (doorOpen_ ? "OPEN" : "CLOSED") << " | "
        << "ALARM: " << (state_ == State::AlarmActive ? "ACTIVE" : "OFF");
    logger_.log(oss.str());
}

// --- Getters for Web Dashboard ---

DoorAlarmFSM::State DoorAlarmFSM::getState() const
{
    std::lock_guard<std::mutex> lock(mutex_);
    return state_;
}

std::string DoorAlarmFSM::getStateString() const
{
    std::lock_guard<std::mutex> lock(mutex_);
    return toString(state_);
}

bool DoorAlarmFSM::isDoorOpen() const
{
    std::lock_guard<std::mutex> lock(mutex_);
    return doorOpen_;
}

bool DoorAlarmFSM::isAlarmActive() const
{
    std::lock_guard<std::mutex> lock(mutex_);
    return state_ == State::AlarmActive;
}

std::optional<DoorAlarmFSM::Clock::time_point> DoorAlarmFSM::getVerificationDeadline() const
{
    std::lock_guard<std::mutex> lock(mutex_);
    return verificationDeadline_;
}

void DoorAlarmFSM::setAuthorizationWindow(std::chrono::milliseconds window)
{
    std::lock_guard<std::mutex> lock(mutex_);
    authorizationWindow_ = window;
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