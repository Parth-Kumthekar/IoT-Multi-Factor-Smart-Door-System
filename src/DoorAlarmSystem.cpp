#include "DoorAlarmSystem.h"

DoorAlarmSystem::DoorAlarmSystem()
    : fsm_(alarmManager_, logger_)
{
}

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
    logger_.log("SYSTEM: FSM initial state = " + DoorAlarmFSM::toString(fsm_.getState()));
}

void DoorAlarmSystem::stop()
{
    if (!running_)
    {
        return;
    }

    running_ = false;
    postEvent(EventType::Shutdown, "main");
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

        fsm_.handleEvent(event);
    }
}

void DoorAlarmSystem::timerLoop()
{
    while (running_)
    {
        std::this_thread::sleep_for(Ms(50));

        const auto state = fsm_.getState();
        const auto deadline = fsm_.getVerificationDeadline();

        if (state == DoorAlarmFSM::State::PendingVerification && deadline.has_value())
        {
            if (Clock::now() >= deadline.value())
            {
                postEvent(EventType::VerificationTimeout, "timer");
                std::this_thread::sleep_for(Ms(100));
            }
        }
    }
}