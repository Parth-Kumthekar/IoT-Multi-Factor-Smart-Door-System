#pragma once
#include <string>

class NFCReader {
public:
    NFCReader();
    std::string readUID();
};