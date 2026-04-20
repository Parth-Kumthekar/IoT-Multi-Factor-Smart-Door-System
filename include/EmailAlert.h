#pragma once
#include <string>

/**
 * @class EmailAlert
 * @brief Handles SMTP communication for sending security notifications.
 * * This class encapsulates the credentials and server settings required to 
 * dispatch emails. It is typically utilized by the AlarmManager to notify 
 * administrators when a security breach is detected.
 */
class EmailAlert {
public:
    /**
     * @brief Default Constructor.
     * @details Uses the hardcoded Team 22 credentials defined in EmailAlert.cpp.
     */
    EmailAlert();

    /**
     * @brief Parameterized Constructor.
     * @details Allows overriding credentials at runtime if needed.
     * * @param smtpServer The address of the outgoing mail server.
     * @param smtpPort The port number (usually 587 for STARTTLS or 465 for SSL).
     * @param senderEmail The email address used to authenticate and send the alert.
     * @param senderPassword The password or app-specific token for the sender account.
     * @param recipientEmail The destination address where alerts will be sent.
     */
    EmailAlert(const std::string& smtpServer,
               int smtpPort,
               const std::string& senderEmail,
               const std::string& senderPassword,
               const std::string& recipientEmail);

    /**
     * @brief Main sending function called by AlarmManager.
     * @param subject The subject line of the email.
     * @param body The main content/reason for the alert.
     * @return true If the email was successfully handed off to the SMTP server.
     * @return false If authentication failed or the server was unreachable.
     */
    bool send(const std::string& subject, const std::string& body);

    /**
     * @brief Validates if the SMTP settings are populated.
     * @return true if all required fields are non-empty.
     */
    bool isConfigured() const;

private:
    /// The SMTP server hostname.
    std::string smtpServer_;
    /// The connection port for the mail server.
    int smtpPort_;
    /// The authorized sender account.
    std::string senderEmail_;
    /// Credentials for the sender account.
    std::string senderPassword_;
    /// The target recipient for all system alerts.
    std::string recipientEmail_;
};
