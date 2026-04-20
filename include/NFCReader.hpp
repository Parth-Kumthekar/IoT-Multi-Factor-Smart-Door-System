#pragma once
#include <string>

/**
 * @class NFCReader
 * @brief Hardware interface for the Near Field Communication (NFC) sensor.
 * * This class manages the low-level serial communication with the NFC module
 * via the Raspberry Pi's UART interface. It facilitates the retrieval of 
 * Unique Identifiers (UIDs) from physical tags, which serve as the primary 
 * credentials for the Door Alarm System.
 */
class NFCReader {
public:
    /**
     * @brief Constructs the reader and assigns the target serial port.
     * @param port The Linux device path for the serial interface (default: "/dev/ttyAMA0").
     */
    NFCReader(const std::string &port = "/dev/ttyAMA0");

    /**
     * @brief Opens the serial device and configures baud rate and parity settings.
     * @return true if the hardware is initialized and ready for communication.
     */
    bool init();

    /**
     * @brief Performs a blocking or timed-out read operation to capture a tag UID.
     * * This method communicates with the NFC controller to poll for the presence 
     * of a tag and returns the hexadecimal UID string if successful.
     * @return std::string The UID of the detected tag, or an empty string if no tag is present.
     */
    std::string readUID();

private:
    /** @brief File descriptor for the opened serial port. */
    int fd;

    /** @brief The system path for the UART device. */
    std::string port;
};