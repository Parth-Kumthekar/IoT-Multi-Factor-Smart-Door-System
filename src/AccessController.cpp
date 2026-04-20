#include "AccessController.hpp"

/**
 * @brief Performs a simple lookup in the allowed UIDs vector.
 * @details Iterates linearly through the internal whitelist to find a match 
 * for the provided unique identifier.
 * * @param uid The unique identifier string collected from the hardware reader.
 * @return true if the UID exists in the authorized list.
 * @return false if the UID is unrecognized.
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
