#include "AccessController.hpp"
#include <algorithm>
#include <cctype>

static std::string normalize(std::string s) {
    s.erase(std::remove_if(s.begin(), s.end(),
        [](char c){
            return c == ' ' || c == '\n' || c == '\r' || c == '\t';
        }), s.end());

    std::transform(s.begin(), s.end(), s.begin(), ::toupper);
    return s;
}

AccessController::AccessController() {
    validUIDs.insert("040ADB8A7111");
    validUIDs.insert("04361B6ABB77");
}

bool AccessController::isAuthorized(const std::string& uidRaw) {

    std::string uid = normalize(uidRaw);

    for (auto &u : validUIDs) {
        if (u == uid) return true;
    }

    return false;
}