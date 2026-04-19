#include "NFCReader.hpp"

#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <cstring>
#include <iostream>

NFCReader::NFCReader(const std::string& dev, int baud)
    : uart_fd(-1), device(dev), baudrate(baud) {}

NFCReader::~NFCReader() {
    if (uart_fd >= 0)
        close(uart_fd);
}

bool NFCReader::init() {
    uart_fd = open(device.c_str(), O_RDWR | O_NOCTTY | O_NDELAY);
    if (uart_fd < 0) {
        std::cerr << "Failed to open UART\n";
        return false;
    }

    return configureUART();
}

bool NFCReader::configureUART() {
    struct termios options;
    tcgetattr(uart_fd, &options);

    cfsetispeed(&options, B9600);
    cfsetospeed(&options, B9600);

    options.c_cflag |= (CLOCAL | CREAD);
    options.c_cflag &= ~PARENB;
    options.c_cflag &= ~CSTOPB;
    options.c_cflag &= ~CSIZE;
    options.c_cflag |= CS8;

    options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
    options.c_iflag &= ~(IXON | IXOFF | IXANY);
    options.c_oflag &= ~OPOST;

    tcsetattr(uart_fd, TCSANOW, &options);

    return true;
}

std::string NFCReader::readUID() {
    char buffer[256];
    memset(buffer, 0, sizeof(buffer));

    int len = read(uart_fd, buffer, sizeof(buffer) - 1);

    if (len > 0) {
        std::string uid(buffer);
        
        // remove newline if present
        uid.erase(uid.find_last_not_of("\r\n") + 1);

        return uid;
    }

    return "";
}