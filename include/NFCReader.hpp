#pragma once
#include <string>

/**
 * @class NFCReader
 * @brief Hardware interface for the Near Field Communication (NFC) sensor.
 */
class NFCReader {
public:
    /**
     * @brief Constructs the reader and assigns the target serial port.
     */
    NFCReader(const std::string &port = "/dev/ttyAMA0");

    /**
     * @brief Opens the serial device.
     */
    bool init();

    /**
     * @brief Reads a tag UID.
     */
    std::string readUID();

private:
    /** * FIX: Swapped declaration order to match the constructor 
     * initialization list and satisfy the -Wreorder compiler warning.
     */
    std::string port;

    /** @brief File descriptor for the opened serial port. */
    int fd;
};