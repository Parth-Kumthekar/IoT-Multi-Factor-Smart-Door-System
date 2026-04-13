#include "NFCReader.hpp"
#include <iostream>

NFCReader::NFCReader() {}

std::string NFCReader::readUID() {
    // Simulate reading UID
    std::string uid;
    std::cout << "Scan NFC card (enter UID): ";
    std::cin >> uid;
    return uid;
}