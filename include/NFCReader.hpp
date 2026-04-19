#pragma once
#include <string>

class NFCReader {
public:
    NFCReader(const std::string &port = "/dev/ttyAMA0");
    bool init();
    std::string readUID();

private:
    int fd;
    std::string port;
};