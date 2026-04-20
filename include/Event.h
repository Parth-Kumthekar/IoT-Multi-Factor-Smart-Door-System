#ifndef EVENT_H
#define EVENT_H

#include <chrono>
#include <string>

/**
 * @enum EventType
 * @brief Defines all possible stimuli that can trigger a state change in the FSM.
 * * This enumeration categorizes hardware interrupts, user inputs, and system 
 * timeouts into discrete types for centralized processing.
 */
enum class EventType
{
    ArmSystem,           ///< Signal to enter Armed state.
    DisarmSystem,        ///< Signal to enter Disarmed state.
    DoorOpened,          ///< Triggered by Reed Switch (GPIO Interrupt).
    DoorClosed,          ///< Triggered by Reed Switch (GPIO Interrupt).
    AuthorizedByNfc,     ///< Valid credential detected via NFCReader.
    AuthorizedByApp,     ///< Access granted via Web API/HTTP.
    VerificationTimeout, ///< Software timer expired during pending state.
    PrintStatus,         ///< Debug/CLI request for system state.
    Shutdown             ///< Signal to stop all threads and exit safely.
};

/**
 * @struct Event
 * @brief Data packet containing state-change information and metadata.
 * * Objects of this struct are passed through the EventQueue to the DoorAlarmFSM.
 * Using a structured event format ensures "Safe use of data management" 
 * as required by the School of Engineering.
 */
struct Event
{
    /** @brief The category of the event occurring. */
    EventType type;

    /** @brief Description of the origin (e.g., "GPIO_PIN_18", "NFC_UID_040ADB8A"). */
    std::string source;

    /** @brief High-resolution wall-clock time when the event was generated. */
    std::chrono::system_clock::time_point timestamp;

    /**
     * @brief Constructs a new Event and automatically captures the current time.
     * @param t The EventType to categorize this event.
     * @param s The string source identifier (defaults to "system").
     */
    Event(EventType t, std::string s = "system")
        : type(t), source(std::move(s)), timestamp(std::chrono::system_clock::now())
    {
    }
};

#endif