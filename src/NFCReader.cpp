#include "NFCReader.hpp"
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <algorithm>
#include <cctype>

NFCReader::NFCReader(const std::string& dev)
    : device(dev), fd(-1) {}

bool NFCReader::init() {
    fd = open(device.c_str(), O_RDWR | O_NOCTTY);

    if (fd < 0) return false;

    struct termios tty{};
    tcgetattr(fd, &tty);

    cfsetispeed(&tty, B9600);
    cfsetospeed(&tty, B9600);

    tty.c_cflag |= (CLOCAL | CREAD);
    tty.c_cflag &= ~CSIZE;
    tty.c_cflag |= CS8;

    tty.c_cflag &= ~PARENB;
    tty.c_cflag &= ~CSTOPB;

    tcsetattr(fd, TCSANOW, &tty);

    return true;
}

std::string NFCReader::readUID() {
    char buf[128] = {0};
    int n = read(fd, buf, sizeof(buf));

    if (n <= 0) return "";

    std::string raw(buf, n);

    // Keep ONLY hex characters
    std::string uid;
    for (char c : raw) {
        if (std::isxdigit(static_cast<unsigned char>(c))) {
            uid += std::toupper(c);
        }
    }

    // Typical NFC UID length check
    if (uid.length() >= 8) {
        return uid;
    }

    return "";
}