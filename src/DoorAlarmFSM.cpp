#include "DoorAlarmFSM.h"
#include <sstream>

/**
 * @brief Construct a new Door Alarm FSM.
 * @details Initializes the system to the ArmedIdle state by default with a 5-second verification window.
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
 * @brief Top-level event dispatcher.
 * @details Thread-safely routes incoming events to their specific internal handler functions.
 * @param event The Event object containing the type and source of the signal.
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
        logger_.log("FSM: Received unhandled event type.");
        break;
    }
}

/**
 * @brief Handles the physical opening of the door.
 * @details If the system is Armed, transitions the state to PendingVerification 
 * and starts the countdown timer.
 */
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

/**
 * @brief Handles the physical closing of the door.
 * @details Performs an "Auto-Rearm" by returning the system to ArmedIdle 
 * and clearing any active alarms or verification timers.
 */
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

/**
 * @brief Validates an authorization request (NFC or App).
 * @details Prevents authorization if the door is already open. If valid, 
 * transitions to AuthorizedEntry and silences any active alarms.
 */
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

/**
 * @brief Triggered when the grace period for door verification expires.
 * @details Transitions the system to AlarmActive and triggers the physical hardware alarm.
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

/**
 * @brief Arms the system.
 * @details Moves the system from Disarmed/Authorized back to monitoring mode (ArmedIdle).
 */
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

/**
 * @brief Disarms the system.
 * @details Moves the system to Disarmed state and clears any pending alarms.
 */
void DoorAlarmFSM::handleDisarm(const std::string& source)
{
    state_ = State::Disarmed;
    clearAuthorizationWindow();
    alarmManager_.clearAlarm();
    logger_.log("FSM: System DISARMED by " + source);
}

// --- Getters & Status ---

/** @brief Public thread-safe method to output system status to the logger. */
void DoorAlarmFSM::printStatus() {
    std::lock_guard<std::mutex> lock(mutex_);
    printStatusInternal();
}

/** @brief Internal non-locking status printer. */
void DoorAlarmFSM::printStatusInternal() {
    std::ostringstream oss;
    oss << "[STATUS] State: " << toString(state_) 
        << " | Door: " << (doorOpen_ ? "OPEN" : "CLOSED")
        << " | Alarm: " << (state_ == State::AlarmActive ? "ACTIVE" : "OFF");
    logger_.log(oss.str());
}

/** @brief Thread-safe getter for the current state. */
DoorAlarmFSM::State DoorAlarmFSM::getState() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return state_;
}

/** @brief Thread-safe getter for the current state as a string. */
std::string DoorAlarmFSM::getStateString() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return toString(state_);
}

/** @brief Thread-safe check for door status. */
bool DoorAlarmFSM::isDoorOpen() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return doorOpen_;
}

/** @brief Thread-safe check for alarm status. */
bool DoorAlarmFSM::isAlarmActive() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return state_ == State::AlarmActive;
}

/** @brief Thread-safe getter for the verification deadline. */
std::optional<DoorAlarmFSM::Clock::time_point> DoorAlarmFSM::getVerificationDeadline() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return verificationDeadline_;
}

/** @brief Updates the duration of the verification window. */
void DoorAlarmFSM::setAuthorizationWindow(std::chrono::milliseconds window) {
    std::lock_guard<std::mutex> lock(mutex_);
    authorizationWindow_ = window;
}

/** @brief Internal helper to reset the deadline optional. */
void DoorAlarmFSM::clearAuthorizationWindow() {
    verificationDeadline_.reset();
}

/**
 * @brief Converts a State enum to string for logging and API.
 * @param state The state to convert.
 * @return std::string The human-readable name of the state.
 */
std::string DoorAlarmFSM::toString(State state) {
    switch (state) {
        case State::Disarmed:            return "Disarmed";
        case State::ArmedIdle:          return "ArmedIdle";
        case State::PendingVerification: return "PendingVerification";
        case State::AuthorizedEntry:    return "AuthorizedEntry";
        case State::AlarmActive:        return "AlarmActive";
        case State::Fault:              return "Fault";
        default:                        return "Unknown";
    }
}
