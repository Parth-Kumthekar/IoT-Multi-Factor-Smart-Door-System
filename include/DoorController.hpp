#pragma once
#include "gpiopin.hpp"
#include "AccessController.hpp"
#include "OutputController.hpp"
#include "NFCReader.hpp"
#include <functional>

/**
 * @class DoorController
 * @brief Manages low-level hardware interactions for the physical door interface.
 * * This class encapsulates the GPIO interactions for the reed switch and coordinates
 * with the NFC reader and output controllers. It facilitates event-driven 
 * hardware responses by utilizing interrupt-style callbacks when physical 
 * sensors change state.
 */
class DoorController {
public:
    /**
     * @brief Constructs the controller with references to required hardware and logic modules.
     * @param reedPin The GPIO pin number assigned to the magnetic reed switch.
     * @param ac Reference to the AccessController for credential verification.
     * @param oc Reference to the OutputController for driving locks/leds.
     * @param nfc Reference to the NFCReader hardware module.
     */
    DoorController(int reedPin, AccessController& ac, OutputController& oc, NFCReader& nfc);

    /**
     * @brief Configures GPIO modes and attaches interrupt service routines (ISRs).
     */
    void initialize();

    /**
     * @brief Safely detaches hardware interrupts and places outputs in a secure state.
     */
    void stop();

private:
    /** @brief GPIO interface for the door's magnetic contact sensor. */
    GPIOPin reedSwitch;

    /** @brief Reference to authorization logic (Single Responsibility Principle). */
    AccessController& accessController;

    /** @brief Reference to hardware output drivers (Solenoid/Buzzer). */
    OutputController& outputController;

    /** @brief Reference to the NFC hardware interface. */
    NFCReader& nfcReader;

    /**
     * @brief Internal callback triggered by the GPIOPin interrupt when the door opens.
     * * This implements the "Event Driven" requirement of the coursework by 
     * responding to hardware signals without CPU polling.
     */
    void onDoorOpen();
};