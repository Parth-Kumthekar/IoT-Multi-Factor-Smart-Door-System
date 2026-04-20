#pragma once
#include <string>

/**
 * @class NFCReader
 * @brief Interfaces with a physical NFC reader module via serial communication.
 * * This class handles the low-level serial (UART) connection to the NFC hardware,
 * allowing the system to initialize the peripheral and poll for scanned 
 * passive tags (UIDs).
 */
class NFCReader {
public:
    /**
     * @brief Construct a new NFCReader object.
     * @param port The filesystem path to the serial device (e.g., "/dev/ttyAMA0").
     */
    NFCReader(const std::string &port = "/dev/ttyAMA0");

    /**
     * @brief Initializes the serial port and the NFC hardware module.
     * @details Sets up baud rate, parity, and stop bits required for communication.
     * @return true If the port was successfully opened and the hardware responded.
     * @return false If the device could not be reached or initialized.
     */
    bool init();

    /**
     * @brief Polls the reader for a scanned tag.
     * @details This is typically a blocking or semi-blocking call that waits for 
     * a tag to enter the reader's induction field.
     * @return std::string The UID of the tag as a hex string, or an empty string if no tag is found.
     */
    std::string readUID();

private:
    /// File descriptor for the opened serial port.
    int fd;

    /// The string path to the serial device.
    std::string port;
};
