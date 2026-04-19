#include "NFCReader.hpp"
#include <fcntl.h>
#include <unistd.h>
#include <iostream>

NFCReader::NFCReader(const std::string &p)
    : port(p), fd(-1) {}

bool NFCReader::init()
{
    fd = open(port.c_str(), O_RDONLY | O_NOCTTY);
    return fd >= 0;
}

std::string NFCReader::readUID()
{
    char buf[64] = {0};

    int n = read(fd, buf, sizeof(buf));
    if (n <= 0) return "";

    std::string s(buf);

    // clean junk
    s.erase(remove(s.begin(), s.end(), '\n'), s.end());
    s.erase(remove(s.begin(), s.end(), '\r'), s.end());

    return s;
}