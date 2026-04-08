#pragma once
#include "NFCReader.hpp"
#include "OutputController.hpp"

class AccessController {
private:
    NFCReader nfc;
    OutputController output;

    const std::string validUID = "1234abcd";

public:
    void init();
    void run();
};