#include "DoorAlarmSystem.h"
#include <iostream>
#include <sstream>

DoorAlarmSystem::DoorAlarmSystem() = default;

DoorAlarmSystem::~DoorAlarmSystem()
{
    stop();
}

void DoorAlarmSystem::start()
{
    if (running_)
    {
        return;
    }

    running_ = true;

    logger_.start();
    alarmManager_.start(logger_);

    controlThread_ = std::thread(&DoorAlarmSystem::controlLoop, this);
    timerThread_ = std::thread(&DoorAlarmSystem::timerLoop, this);

    logger_.log("SYSTEM: started.");
    logger_.log("SYSTEM: initial state = " + toString(state_));
}

void DoorAlarmSystem::stop()
{
    if (!running_)
    {
        return;
    }

    running_ = false;

    // Push a shutdown event so the control loop can exit cleanly.
    postEvent(EventType::Shutdown, "main");

    // Also shut down the queue to wake any waiting thread safely.
    eventQueue_.shutdown();

    if (controlThread_.joinable())
    {
        controlThread_.join();
    }

    if (timerThread_.joinable())
    {
        timerThread_.join();
    }

    alarmManager_.stop();
    logger_.log("SYSTEM: stopped.");
    logger_.stop();
}

void DoorAlarmSystem::postEvent(EventType type, const std::string& source)
{
    eventQueue_.push(Event(type, source));
}

std::string DoorAlarmSystem::toString(SystemState state)
{
    switch (state)
    {
    case SystemState::Disarmed:
        return "Disarmed";
    case SystemState::ArmedIdle:
        return "ArmedIdle";
    case SystemState::PendingVerification:
        return "PendingVerification";
    case SystemState::AuthorizedEntry:
        return "AuthorizedEntry";
    case SystemState::AlarmActive:
        return "AlarmActive";
    case SystemState::Fault:
        return "Fault";
    default:
        return "Unknown";
    }
}

void DoorAlarmSystem::controlLoop()
{
    while (true)
    {
        Event event(EventType::PrintStatus);

        if (!eventQueue_.waitAndPop(event))
        {
            logger_.log("CONTROL: event queue shutdown detected.");
            break;
        }

        if (event.type == EventType::Shutdown)
        {
            logger_.log("CONTROL: shutdown event received.");
            break;
        }

        handleEvent(event);
    }
}

void DoorAlarmSystem::timerLoop()
{
    while (running_)
    {
        std::this_thread::sleep_for(Ms(50));

        bool shouldTimeout = false;
        {
            std::lock_guard<std::mutex> lock(stateMutex_);
            if (state_ == SystemState::PendingVerification && verificationDeadline_.has_value())
            {
                if (Clock::now() >= verificationDeadline_.value())
                {
                    shouldTimeout = true;
                }
            }
        }

        if (shouldTimeout)
        {
            postEvent(EventType::VerificationTimeout, "timer");
            std::this_thread::sleep_for(Ms(100));
        }
    }
}

void DoorAlarmSystem::handleEvent(const Event& event)
{
    std::lock_guard<std::mutex> lock(stateMutex_);

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

void DoorAlarmSystem::handleArm(const std::string& source)
{
    if (state_ == SystemState::Disarmed || state_ == SystemState::AuthorizedEntry)
    {
        state_ = SystemState::ArmedIdle;
        doorOpen_ = false;
        clearAuthorizationWindow();
        alarmManager_.clearAlarm();

        logger_.log("CONTROL: system ARMED by " + source + ". New state = " + toString(state_));
    }
    else
    {
        logger_.log("CONTROL: ARM ignored in state " + toString(state_) + ".");
    }
}

void DoorAlarmSystem::handleDisarm(const std::string& source)
{
    state_ = SystemState::Disarmed;
    clearAuthorizationWindow();
    alarmManager_.clearAlarm();

    logger_.log("CONTROL: system DISARMED by " + source + ". New state = " + toString(state_));
}

void DoorAlarmSystem::handleDoorOpened(const std::string& source)
{
    doorOpen_ = true;
    logger_.log("CONTROL: door opened from " + source + ".");

    if (state_ == SystemState::Disarmed)
    {
        logger_.log("CONTROL: door open allowed because system is disarmed.");
        return;
    }

    if (state_ == SystemState::ArmedIdle)
    {
        state_ = SystemState::PendingVerification;
        verificationDeadline_ = Clock::now() + authorizationWindow_;

        logger_.log(
            "CONTROL: state -> PendingVerification. Waiting for NFC/App auth for "
            + std::to_string(authorizationWindow_.count()) + " ms.");
        return;
    }

    if (state_ == SystemState::AlarmActive)
    {
        logger_.log("CONTROL: door opened while alarm already active.");
        return;
    }

    logger_.log("CONTROL: door open event received in state " + toString(state_) + ".");
}

void DoorAlarmSystem::handleDoorClosed(const std::string& source)
{
    doorOpen_ = false;
    logger_.log("CONTROL: door closed from " + source + ".");

    if (state_ == SystemState::AuthorizedEntry)
    {
        state_ = SystemState::ArmedIdle;
        clearAuthorizationWindow();
        logger_.log("CONTROL: authorized entry finished. State -> ArmedIdle.");
    }
}

void DoorAlarmSystem::handleAuthorization(const Event& event)
{
    const std::string method =
        (event.type == EventType::AuthorizedByNfc) ? "NFC" : "APP";

    if (state_ == SystemState::PendingVerification && verificationDeadline_.has_value())
    {
        if (Clock::now() <= verificationDeadline_.value())
        {
            state_ = SystemState::AuthorizedEntry;
            clearAuthorizationWindow();
            alarmManager_.clearAlarm();

            logger_.log("CONTROL: valid authorization by " + method + ". State -> AuthorizedEntry.");
        }
        else
        {
            logger_.log("CONTROL: late authorization by " + method + " ignored.");
        }
        return;
    }

    if (state_ == SystemState::ArmedIdle)
    {
        logger_.log(
            "CONTROL: pre-authorization by " + method
            + " received, but this demo only accepts it after door opens.");
        return;
    }

    if (state_ == SystemState::AlarmActive)
    {
        logger_.log(
            "CONTROL: authorization by " + method
            + " ignored because alarm is already active. Use disarm.");
        return;
    }

    logger_.log("CONTROL: authorization by " + method + " ignored in state " + toString(state_) + ".");
}

void DoorAlarmSystem::handleVerificationTimeout(const std::string& source)
{
    if (state_ == SystemState::PendingVerification)
    {
        state_ = SystemState::AlarmActive;
        clearAuthorizationWindow();
        alarmManager_.triggerAlarm(
            "Unauthorized door opening (timeout, source=" + source + ")");

        logger_.log("CONTROL: verification timeout. State -> AlarmActive.");
    }
}

void DoorAlarmSystem::printStatus()
{
    std::ostringstream oss;
    oss << "STATUS: "
        << "state=" << toString(state_)
        << ", doorOpen=" << (doorOpen_ ? "true" : "false")
        << ", alarm=" << (alarmManager_.isAlarmActive() ? "ON" : "OFF");

    logger_.log(oss.str());
}

void DoorAlarmSystem::clearAuthorizationWindow()
{
    verificationDeadline_.reset();
}