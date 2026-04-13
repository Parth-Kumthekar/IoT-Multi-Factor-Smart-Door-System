#pragma once
#include <string>
#include <vector>

class AccessController {
public:
    AccessController(const std::vector<std::string>& authorizedUIDs);
    bool isAuthorized(const std::string& uid) const;

private:
    std::vector<std::string> authorizedUIDs;
};