#pragma once

#include <vector>
#include <cstdint>   // For uint8_t
#include <string>
#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>

/**
 * @brief NFCReader class for reading NFC cards via I2C interface
 */
class NFCReader {
public:
    /**
     * @param bus I2C bus number (default 1)
     * @param address I2C device address (default 0x24)
     */
    NFCReader(int bus = 1, int address = 0x24);
    ~NFCReader();

    /**
     * @brief Send command to NFC device
     * @param cmd Vector of bytes representing the command
     * @return true if write succeeded
     */
    bool writeCommand(const std::vector<uint8_t>& cmd);

    /**
     * @brief Read response from NFC device
     * @param len Number of bytes to read
     * @return Vector of bytes received
     */
    std::vector<uint8_t> readResponse(int len);

    /**
     * @brief Verify if the NFC card is authorized
     * @return true if card is authorized
     */
    bool verifyCard();

private:
    int i2c_fd;       // File descriptor for I2C device
    int i2c_bus;      // I2C bus number
    int i2c_address;  // I2C device address

    bool openDevice();   // Open I2C device
    void closeDevice();  // Close I2C device
};