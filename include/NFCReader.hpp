#pragma once

#include <string>

class NFCReader {
public:
    NFCReader(const std::string& dev = "/dev/ttyAMA0");

    bool init();
    std::string readUID();

private:
    int fd;
    std::string device;
};