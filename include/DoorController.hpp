#pragma once
#include "gpiopin.hpp"
#include "AccessController.hpp"
#include "OutputController.hpp"
#include "NFCReader.hpp"
#include <functional>

/**
 * @class DoorController
 * @brief High-level hardware abstraction for managing physical door components.
 * * This class orchestrates the interaction between the magnetic reed switch, 
 * the NFC reader, and the output signals. It serves as the direct interface 
 * to the physical entry point.
 */
class DoorController {
public:
    /**
     * @brief Construct a new Door Controller object.
     * * @param reedPin The GPIO pin number assigned to the reed switch sensor.
     * @param ac Reference to the logic handling access permissions.
     * @param oc Reference to the hardware signaling (Siren/LEDs).
     * @param nfc Reference to the physical NFC reader hardware.
     */
    DoorController(int reedPin, AccessController& ac, OutputController& oc, NFCReader& nfc);

    /**
     * @brief Configures hardware pins and prepares sensors for monitoring.
     * @details Should be called after instantiation to ensure GPIO and NFC bus are ready.
     */
    void initialize();

    /**
     * @brief Safely deactivates hardware monitoring and releases pin resources.
     */
    void stop();

private:
    /// GPIO pin object representing the magnetic door sensor.
    GPIOPin reedSwitch;

    /// Reference to the shared access validation logic.
    AccessController& accessController;

    /// Reference to the shared output hardware controller.
    OutputController& outputController;

    /// Reference to the shared NFC reader interface.
    NFCReader& nfcReader;

    /**
     * @brief Internal callback triggered when the reed switch detects a state change.
     */
    void onDoorOpen();
};
