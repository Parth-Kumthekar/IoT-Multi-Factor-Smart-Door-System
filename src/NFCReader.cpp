#include "NFCReader.hpp"
#include <fcntl.h>
#include <unistd.h>
#include <iostream>
#include <algorithm>

/**
 * @brief Constructor for the NFC hardware interface.
 * @param p The device path (e.g., "/dev/ttyAMA0" for RPi hardware UART).
 */
NFCReader::NFCReader(const std::string &p)
    : port(p), fd(-1) {}

/**
 * @brief Opens the serial port in Read-Only mode.
 * @details Uses O_NOCTTY to prevent the port from becoming the process's 
 * controlling terminal, ensuring stability.
 * @return true if the file descriptor was successfully acquired.
 */
bool NFCReader::init()
{
    fd = open(port.c_str(), O_RDONLY | O_NOCTTY);
    if (fd < 0) {
        // Log to stderr for system diagnostics
        std::cerr << "[NFC] Failed to open port: " << port << std::endl;
    }
    return fd >= 0;
}

/**
 * @brief Reads and sanitizes a UID from the serial buffer.
 * @details This is a blocking read. In the multi-threaded DoorAlarmSystem, 
 * this runs in its own nfcThread_ to prevent UI/Logic stuttering.
 * @return std::string The cleaned UID or an empty string on read failure/timeout.
 */
std::string NFCReader::readUID()
{
    char buf[64] = {0};

    // Read raw data from the serial driver
    int n = read(fd, buf, sizeof(buf));
    if (n <= 0) return "";

    // Convert to string safely using the actual number of bytes read
    std::string s(buf, n); 

    // Sanitization: Remove non-printable CRLF characters often sent by NFC modules
    s.erase(std::remove(s.begin(), s.end(), '\n'), s.end());
    s.erase(std::remove(s.begin(), s.end(), '\r'), s.end());

    return s;
}