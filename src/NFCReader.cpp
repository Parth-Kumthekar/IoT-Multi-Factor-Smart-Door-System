#include "NFCReader.hpp"
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <iostream>
#include <sstream>
#include <iomanip>

#define PN532_COMMAND_INLISTPASSIVETARGET 0x4A

NFCReader::NFCReader() : i2c_fd(-1) {}

NFCReader::~NFCReader() {
    if (i2c_fd >= 0) close(i2c_fd);
}

bool NFCReader::init() {
    i2c_fd = open("/dev/i2c-1", O_RDWR);
    if (i2c_fd < 0) return false;

    if (ioctl(i2c_fd, I2C_SLAVE, addr) < 0) return false;

    return true;
}

bool NFCReader::writeCommand(const std::vector<uint8_t>& cmd) {
    return write(i2c_fd, cmd.data(), cmd.size()) == (int)cmd.size();
}

std::vector<uint8_t> NFCReader::readResponse(int len) {
    std::vector<uint8_t> buffer(len);
    read(i2c_fd, buffer.data(), len);
    return buffer;
}

std::string NFCReader::readUID() {
    std::vector<uint8_t> cmd = {0x00, 0x00, 0xFF, 0x04, 0xFC, 0xD4, 0x4A, 0x01, 0x00, 0xE1, 0x00};

    writeCommand(cmd);
    usleep(50000);

    auto resp = readResponse(32);

    int uid_len = resp[12];

    std::stringstream ss;
    for (int i = 0; i < uid_len; i++) {
        ss << std::hex << std::setw(2) << std::setfill('0')
           << (int)resp[13 + i];
    }

    return ss.str();
}