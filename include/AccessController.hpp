#pragma once

#include <set>
#include <string>

class AccessController {
public:
    AccessController();

    bool isAuthorized(const std::string& uid);

private:
    std::set<std::string> validUIDs;
};