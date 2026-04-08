#include "NFCReader.hpp"

NFCReader::NFCReader(int bus, int address)
    : i2c_fd(-1), i2c_bus(bus), i2c_address(address) {}

NFCReader::~NFCReader() {
    closeDevice();
}

bool NFCReader::openDevice() {
    std::string device = "/dev/i2c-" + std::to_string(i2c_bus);
    i2c_fd = ::open(device.c_str(), O_RDWR);
    if (i2c_fd < 0) {
        std::cerr << "Failed to open I2C device\n";
        return false;
    }
    if (ioctl(i2c_fd, I2C_SLAVE, i2c_address) < 0) {
        std::cerr << "Failed to set I2C address\n";
        ::close(i2c_fd);
        i2c_fd = -1;
        return false;
    }
    return true;
}

void NFCReader::closeDevice() {
    if (i2c_fd >= 0) {
        ::close(i2c_fd);
        i2c_fd = -1;
    }
}

bool NFCReader::writeCommand(const std::vector<uint8_t>& cmd) {
    if (i2c_fd < 0 && !openDevice()) return false;
    ssize_t res = ::write(i2c_fd, cmd.data(), cmd.size());
    return res == (ssize_t)cmd.size();
}

std::vector<uint8_t> NFCReader::readResponse(int len) {
    std::vector<uint8_t> buffer(len);
    if (i2c_fd < 0 && !openDevice()) return {};
    ssize_t res = ::read(i2c_fd, buffer.data(), len);
    if (res != len) buffer.resize(res >= 0 ? res : 0);
    return buffer;
}

bool NFCReader::verifyCard() {
    // Example logic: send a command, check response
    std::vector<uint8_t> cmd = {0x01, 0x02};  // placeholder
    if (!writeCommand(cmd)) return false;
    auto resp = readResponse(4);
    return !resp.empty() && resp[0] == 0x00;  // placeholder check
}

std::string NFCReader::readUID() {
    auto resp = readResponse(8);  // read 8-byte UID
    std::string uid;
    for (auto b : resp) {
        char buf[3];
        snprintf(buf, sizeof(buf), "%02X", b);
        uid += buf;
    }
    return uid;
}