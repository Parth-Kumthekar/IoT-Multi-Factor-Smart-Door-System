#include "NFCReader.hpp"
#include <fcntl.h>
#include <unistd.h>
#include <iostream>
#include <algorithm>

/**
 * @brief Construct a new NFCReader instance.
 * @param p The filesystem path to the serial UART device (e.g., "/dev/ttyAMA0").
 */
NFCReader::NFCReader(const std::string &p)
    : port(p), fd(-1) {}

/**
 * @brief Opens the serial device file.
 * @details Uses the POSIX open() call with O_RDONLY and O_NOCTTY flags.
 * O_NOCTTY ensures the serial port does not become the process's controlling terminal.
 * @return true if the file descriptor was successfully obtained.
 * @return false if the device could not be opened (e.g., permission denied or wrong path).
 */
bool NFCReader::init()
{
    fd = open(port.c_str(), O_RDONLY | O_NOCTTY);
    return fd >= 0;
}

/**
 * @brief Reads a raw UID from the serial buffer and cleans the output.
 * @details Performs a blocking read on the file descriptor. The raw data is then
 * processed to remove common serial "junk" like newline (\\n) and carriage 
 * return (\\r) characters to ensure the UID can be correctly matched by the AccessController.
 * * @return std::string The cleaned UID string, or an empty string if no data was read.
 */
std::string NFCReader::readUID()
{
    char buf[64] = {0};

    // Low-level POSIX read from the serial device
    int n = read(fd, buf, sizeof(buf));
    if (n <= 0) return "";

    // Convert buffer to string based on actual bytes read
    std::string s(buf, n);   

    // Data Sanitization: clean junk characters typically sent by serial modules
    s.erase(std::remove(s.begin(), s.end(), '\n'), s.end());
    s.erase(std::remove(s.begin(), s.end(), '\r'), s.end());

    return s;
}
