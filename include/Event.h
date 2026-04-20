#ifndef EVENT_H
#define EVENT_H

#include <chrono>
#include <string>

/**
 * @enum EventType
 * @brief Stimuli that trigger transitions in the FSM.
 */
enum class EventType
{
    ArmSystem,           ///< Signal to enter Armed state.
    DisarmSystem,        ///< Signal to enter Disarmed state.
    DoorOpened,          ///< Triggered by Reed Switch (Open).
    DoorClosed,          ///< Triggered by Reed Switch (Closed).
    AuthorizedByNfc,     ///< Valid UID detected.
    AuthorizedByApp,     ///< Access granted via Web API.
    VerificationTimeout, ///< Grace period expired.
    PrintStatus,         ///< Debug request.
    Shutdown             ///< Graceful exit signal.
};

/**
 * @struct Event
 * @brief Data packet containing state-change information.
 */
struct Event
{
    /** @brief The category of the event. */
    EventType type;

    /** @brief Description of the origin (e.g., UID or Sensor name). */
    std::string source;

    /** @brief High-resolution wall-clock time of creation. */
    std::chrono::system_clock::time_point timestamp;

    /**
     * @brief Constructs a new Event.
     * @param t The EventType.
     * @param s The source identifier.
     */
    Event(EventType t, std::string s = "system")
        : type(t), 
          source(std::move(s)), 
          timestamp(std::chrono::system_clock::now())
    {
    }
};

#endif