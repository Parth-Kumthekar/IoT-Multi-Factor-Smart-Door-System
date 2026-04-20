#include "AccessController.hpp"
#include <algorithm>

/**
 * @brief Performs a lookup in the authorized UIDs list.
 * * This implementation uses std::any_of for a more expressive, declarative 
 * approach. It maintains O(n) complexity, which is optimal for the 
 * small-scale authorized list defined in the class header.
 */
bool AccessController::check(const std::string &uid) {
    // Check if the provided UID exists within the encapsulated 'allowed' vector
    return std::any_of(allowed.begin(), allowed.end(), [&uid](const std::string& authorized_id) {
        return authorized_id == uid;
    });
}