#include "AccessController.hpp"

bool AccessController::check(const std::string &uid)
{
    for (auto &a : allowed)
        if (a == uid) return true;

    return false;
}