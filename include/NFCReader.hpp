#pragma once
#include <string>
#include <vector>

class NFCReader {
private:
    int i2c_fd;
    const int addr = 0x24;

    bool writeCommand(const std::vector<uint8_t>& cmd);
    std::vector<uint8_t> readResponse(int len);

public:
    NFCReader();
    ~NFCReader();

    bool init();
    std::string readUID();
};