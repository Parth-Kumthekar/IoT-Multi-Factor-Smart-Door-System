#include "DoorAlarmFSM.h"
#include <sstream>

/**
 * @brief Constructor for the FSM logic engine.
 * @details Initializes to ArmedIdle to ensure the system is "secure by default" 
 * upon startup.
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
 * @details Uses a mutex to ensure that state transitions are atomic and 
 * thread-safe across the multi-threaded orchestrator.
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
        // Handled by the orchestrator lifecycle
        break;
    default:
        logger_.log("FSM", "Received unhandled event type.");
        break;
    }
}

/**
 * @brief Transition handler for physical door opening.
 * @details If Armed, initiates the 'Pending' grace period.
 */
void DoorAlarmFSM::handleDoorOpened(const std::string& source)
{
    doorOpen_ = true;
    logger_.log("HARDWARE", "Door OPENED (Source: " + source + ")");

    if (state_ == State::ArmedIdle)
    {
        state_ = State::PendingVerification;
        verificationDeadline_ = Clock::now() + authorizationWindow_;
        logger_.log("SECURITY", "Unauthorized Open. 5s grace period started.");
    }
}

/**
 * @brief Transition handler for physical door closing.
 * @details Automatically re-arms the system to maintain security integrity.
 */
void DoorAlarmFSM::handleDoorClosed(const std::string& source) 
{
    doorOpen_ = false;
    logger_.log("HARDWARE", "Door CLOSED (Source: " + source + ")");
    
    if (state_ == State::AuthorizedEntry || state_ == State::PendingVerification || state_ == State::AlarmActive) {
        state_ = State::ArmedIdle;
        clearAuthorizationWindow();
        alarmManager_.clearAlarm(); 
        logger_.log("SECURITY", "System re-secured. State -> ArmedIdle.");
    }
}

/**
 * @brief Processes authorization requests from NFC or Web API.
 * @details Cancels pending alarms if the credential is valid.
 */
void DoorAlarmFSM::handleAuthorization(const Event& event) 
{
    std::string method = (event.type == EventType::AuthorizedByNfc) ? "NFC" : "APP";

    if (doorOpen_) {
        logger_.log("ACCESS", "Auth via " + method + " REJECTED. Door is already open.");
        return; 
    }

    if (state_ == State::ArmedIdle || state_ == State::PendingVerification) 
    {
        state_ = State::AuthorizedEntry;
        clearAuthorizationWindow();
        alarmManager_.clearAlarm(); 
        logger_.log("ACCESS", "GRANTED via " + method + " (ID: " + event.source + "). Unlocking.");
    }
    else if (state_ == State::AlarmActive) {
        state_ = State::AuthorizedEntry;
        alarmManager_.clearAlarm();
        logger_.log("ACCESS", "Alarm silenced by authorized " + method);
    }
}

/**
 * @brief Handles the expiration of the security grace period.
 */
void DoorAlarmFSM::handleVerificationTimeout(const std::string& source)
{
    if (state_ == State::PendingVerification)
    {
        state_ = State::AlarmActive;
        clearAuthorizationWindow();
        alarmManager_.triggerAlarm("Timeout triggered by: " + source);
        logger_.log("SECURITY", "ALARM: Grace period expired!");
    }
}

// ... Rest of the methods follow the same mutex-locked pattern ...