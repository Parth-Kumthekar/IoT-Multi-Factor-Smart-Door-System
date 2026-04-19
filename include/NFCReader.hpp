#pragma once

#include <string>

class NFCReader {
public:
    NFCReader(const std::string& device = "/dev/serial0", int baudrate = 9600);
    ~NFCReader();

    bool init();
    std::string readUID();   // blocking read

private:
    int uart_fd;
    std::string device;
    int baudrate;

    bool configureUART();
};