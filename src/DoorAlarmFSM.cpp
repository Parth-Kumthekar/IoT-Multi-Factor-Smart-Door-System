#include "DoorAlarmFSM.h"
#include <sstream>

DoorAlarmFSM::DoorAlarmFSM(AlarmManager& alarmManager, AsyncLogger& logger)
    : alarmManager_(alarmManager), logger_(logger)
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
    case State::Disarmed:
        return "Disarmed";
    case State::ArmedIdle:
        return "ArmedIdle";
    case State::PendingVerification:
        return "PendingVerification";
    case State::AuthorizedEntry:
        return "AuthorizedEntry";
    case State::AlarmActive:
        return "AlarmActive";
    case State::Fault:
        return "Fault";
    default:
        return "Unknown";
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
        doorOpen_ = false;
        clearAuthorizationWindow();
        alarmManager_.clearAlarm();
        logger_.log("FSM: system ARMED by " + source + ". State -> " + toString(state_));
    }
    else
    {
        logger_.log("FSM: ARM ignored in state " + toString(state_) + ".");
    }
}

void DoorAlarmFSM::handleDisarm(const std::string& source)
{
    state_ = State::Disarmed;
    clearAuthorizationWindow();
    alarmManager_.clearAlarm();
    logger_.log("FSM: system DISARMED by " + source + ". State -> " + toString(state_));
}

void DoorAlarmFSM::handleDoorOpened(const std::string& source)
{
    doorOpen_ = true;
    logger_.log("FSM: door opened from " + source + ".");

    if (state_ == State::Disarmed)
    {
        logger_.log("FSM: door open allowed because system is disarmed.");
        return;
    }

    if (state_ == State::ArmedIdle)
    {
        state_ = State::PendingVerification;
        verificationDeadline_ = Clock::now() + authorizationWindow_;
        logger_.log("FSM: State -> PendingVerification. Waiting for authorization for "
                    + std::to_string(authorizationWindow_.count()) + " ms.");
        return;
    }

    if (state_ == State::AlarmActive)
    {
        logger_.log("FSM: door opened while alarm already active.");
        return;
    }

    logger_.log("FSM: door open received in state " + toString(state_) + ".");
}

void DoorAlarmFSM::handleDoorClosed(const std::string& source) {
    doorOpen_ = false;
    if (state_ == State::AuthorizedEntry) {
        state_ = State::ArmedIdle; // Automatically re-arms when door shuts
        logger_.log("FSM: Door closed. System re-armed.");
    }
}

void DoorAlarmFSM::handleAuthorization(const Event& event)
{
    const std::string method = (event.type == EventType::AuthorizedByNfc) ? "NFC" : "CAMERA";

    // 1. Check if the system is in a state where access can be granted
    // We allow access from Disarmed, ArmedIdle (Direct Entry), or Pending (Door already open)
    if (state_ == State::PendingVerification || state_ == State::ArmedIdle || state_ == State::Disarmed)
    {
        state_ = State::AuthorizedEntry; 
        clearAuthorizationWindow();
        alarmManager_.clearAlarm();

        logger_.log("FSM: DIRECT ACCESS GRANTED by " + method + ". State -> AuthorizedEntry.");
        
        // Return here because we have successfully handled the event
        return; 
    }

    // 2. Handle cases where the alarm is already going off
    if (state_ == State::AlarmActive)
    {
        logger_.log("FSM: Auth by " + method + " rejected. Alarm is ACTIVE! Reset system first.");
        return;
    }

    // 3. Fallback for any other state (like Fault or already Authorized)
    logger_.log("FSM: Auth by " + method + " ignored in current state: " + toString(state_));
}

void DoorAlarmFSM::handleVerificationTimeout(const std::string& source)
{
    if (state_ == State::PendingVerification)
    {
        state_ = State::AlarmActive;
        clearAuthorizationWindow();
        alarmManager_.triggerAlarm("Unauthorized door opening (timeout, source=" + source + ")");
        logger_.log("FSM: verification timeout. State -> AlarmActive.");
    }
}

void DoorAlarmFSM::printStatus()
{
    std::ostringstream oss;
    oss << "FSM STATUS: "
        << "state=" << toString(state_)
        << ", doorOpen=" << (doorOpen_ ? "true" : "false")
        << ", alarm=" << (alarmManager_.isAlarmActive() ? "ON" : "OFF");

    logger_.log(oss.str());
}

void DoorAlarmFSM::clearAuthorizationWindow()
{
    verificationDeadline_.reset();
}