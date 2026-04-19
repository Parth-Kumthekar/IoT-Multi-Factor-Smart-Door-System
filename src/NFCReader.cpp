#include "NFCReader.hpp"
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <cstring>

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

    tcsetattr(fd, TCSANOW, &tty);

    return true;
}

std::string NFCReader::readUID() {
    char buf[64] = {0};
    int n = read(fd, buf, sizeof(buf));
    if (n > 0) {
        return std::string(buf, n);
    }
    return "";
}