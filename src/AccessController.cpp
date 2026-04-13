#include "AccessController.hpp"
#include <algorithm>

AccessController::AccessController(const std::vector<std::string>& uids) : authorizedUIDs(uids) {}

bool AccessController::isAuthorized(const std::string& uid) const {
    return std::find(authorizedUIDs.begin(), authorizedUIDs.end(), uid) != authorizedUIDs.end();
}