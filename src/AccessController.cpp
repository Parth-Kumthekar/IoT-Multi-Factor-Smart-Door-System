#include "AccessController.hpp"

AccessController::AccessController() {
    validUIDs.insert("040ADB8A7111");
    validUIDs.insert("04361B6ABB77");
}

bool AccessController::isAuthorized(const std::string& uid) {
    return validUIDs.count(uid) > 0;
}