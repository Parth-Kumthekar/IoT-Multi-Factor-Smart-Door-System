#pragma once

#include <vector>
#include <cstdint>
#include <string>
#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>

class NFCReader {
public:
    NFCReader(int bus = 1, int address = 0x24);
    ~NFCReader();

    bool writeCommand(const std::vector<uint8_t>& cmd);
    std::vector<uint8_t> readResponse(int len);
    bool verifyCard();
    std::string readUID();

private:
    int i2c_fd;
    int i2c_bus;
    int i2c_address;

    bool openDevice();
    void closeDevice();
};