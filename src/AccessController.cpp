#include "AccessController.hpp"

/**
 * @brief Performs a simple lookup in the allowed UIDs vector.
 */
bool AccessController::check(const std::string &uid) {
    // Iterate through the authorized list
    for (const auto &authorized_id : allowed) {
        if (authorized_id == uid) {
            return true; // Access Granted
        }
    }
    
    // If we reach here, no match was found
    return false; // Access Denied
}
