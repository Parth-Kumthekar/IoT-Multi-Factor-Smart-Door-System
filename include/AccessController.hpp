#pragma once
#include <string>
#include <vector>

/**
 * @class AccessController
 * @brief Manages and validates UID-based access permissions.
 * * This class maintains an internal whitelist of authorized unique identifiers
 * and provides a mechanism to verify if a given UID has access.
 */
class AccessController {
public:
    /**
     * @brief Validates if a specific UID is in the allowed list.
     * * @param uid The unique identifier string to be checked.
     * @return true If the UID is found in the authorized list.
     * @return false If the UID is not recognized or access is denied.
     */
    bool check(const std::string &uid);

private:
    /** * @brief List of authorized hardware UIDs.
     * @details This list is currently hardcoded with specific hex-string identifiers.
     */
    std::vector<std::string> allowed = {
        "040ADB8A7111",
        "04361B6ABB77"
    };
};
