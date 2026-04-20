#pragma once
#include <string>
#include <vector>

/**
 * @class AccessController
 * @brief Manages the authorization logic for the IoT Door Lock system.
 * * This class encapsulates the "Allowed Users" list and provides a thread-safe 
 * interface to verify NFC Unique Identifiers (UIDs). It follows the SOLID 
 * Single Responsibility Principle by strictly handling access logic.
 */
class AccessController {
public:
    /**
     * @brief Validates an NFC UID against the authorized list.
     * * This method performs a lookup within the internal vector to determine
     * if the scanned credential has permission to unlock the door.
     * * @param uid The Unique Identifier string retrieved from the NFC sensor.
     * @return true if the UID is authorized, false otherwise.
     */
    bool check(const std::string &uid);

private:
    /**
     * @brief List of authorized NFC UIDs.
     * @note In a production environment, this could be moved to an encrypted 
     * local database or the CSV logger.
     */
    std::vector<std::string> allowed = {
        "040ADB8A7111",
        "04361B6ABB77"
    };
};