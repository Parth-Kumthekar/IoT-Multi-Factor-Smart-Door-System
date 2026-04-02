#ifndef EVENT_H
#define EVENT_H

#include <chrono>
#include <string>

enum class EventType
{
    ArmSystem,
    DisarmSystem,
    DoorOpened,
    DoorClosed,
    AuthorizedByNfc,
    AuthorizedByApp,
    VerificationTimeout,
    PrintStatus,
    Shutdown
};

struct Event
{
    EventType type;
    std::string source;
    std::chrono::system_clock::time_point timestamp;

    Event(EventType t, std::string s = "system")
        : type(t), source(std::move(s)), timestamp(std::chrono::system_clock::now())
    {
    }
};

#endif