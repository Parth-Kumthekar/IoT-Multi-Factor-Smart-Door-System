#include "NFCReader.hpp"
#include <fcntl.h>
#include <unistd.h>
#include <iostream>
#include <algorithm>

/**
 * @brief Constructor for the NFC hardware interface.
 * FIX: Initialization list ordered to match the Header declaration.
 */
NFCReader::NFCReader(const std::string &p)
    : port(p), fd(-1) {}

/**
 * @brief Opens the serial port.
 */
bool NFCReader::init()
{
    // O_RDONLY: Read only for security tags
    // O_NOCTTY: Don't let this port control the terminal
    // O_NDELAY: Non-blocking open to prevent the system from hanging if the hardware is disconnected
    fd = open(port.c_str(), O_RDONLY | O_NOCTTY | O_NDELAY);
    
    if (fd < 0) {
        std::cerr << "[NFC ERROR] Could not open " << port << ". Check UART permissions." << std::endl;
        return false;
    }

    // After opening, we clear the NDELAY flag so readUID can block efficiently 
    // inside its dedicated thread without spinning the CPU.
    fcntl(fd, F_SETFL, 0); 
    
    return true;
}

/**
 * @brief Reads and sanitizes a UID from the serial buffer.
 */
std::string NFCReader::readUID()
{
    if (fd < 0) return "";

    char buf[64] = {0};

    // Read raw data from the serial driver
    ssize_t n = read(fd, buf, sizeof(buf) - 1);
    if (n <= 0) return "";

    std::string s(buf, n); 

    // Sanitization: Remove CRLF characters
    s.erase(std::remove(s.begin(), s.end(), '\n'), s.end());
    s.erase(std::remove(s.begin(), s.end(), '\r'), s.end());

    return s;
}