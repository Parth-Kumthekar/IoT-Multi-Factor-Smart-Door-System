#pragma once
#include <string>

/**
 * @class EmailAlert
 * @brief Provides remote notification capabilities via SMTP.
 * * This class encapsulates the network logic required to send security alerts 
 * to a designated recipient. It is used by the AlarmManager to provide 
 * off-site monitoring when a security breach is detected, following the 
 * "High Reliability" requirement of the School of Engineering.
 */
class EmailAlert {
public:
    /**
     * @brief Default Constructor.
     * * Initializes the alert system using pre-configured Team 22 credentials.
     * This ensures the system is "Plug-and-Play" for markers and users.
     */
    EmailAlert();

    /**
     * @brief Parameterized Constructor for custom network configurations.
     * @param smtpServer The address of the outgoing mail server.
     * @param smtpPort The communication port (e.g., 465 or 587).
     * @param senderEmail The system's automated email address.
     * @param senderPassword The secure app-specific password/token.
     * @param recipientEmail The end-user's notification address.
     */
    EmailAlert(const std::string& smtpServer,
               int smtpPort,
               const std::string& senderEmail,
               const std::string& senderPassword,
               const std::string& recipientEmail);

    /**
     * @brief Dispatches an email notification.
     * * This method is called by the AlarmManager. It connects to the SMTP 
     * server and attempts to send the alert body.
     * @param subject The title of the alert (e.g., "SECURITY BREACH").
     * @param body The detailed log of the event causing the alarm.
     * @return true if the email was successfully handed off to the server, false otherwise.
     */
    bool send(const std::string& subject, const std::string& body);

    /**
     * @brief Checks if the SMTP credentials have been properly loaded.
     * @return true if the system is ready to send alerts.
     */
    bool isConfigured() const;

private:
    /** @brief The SMTP relay hostname. */
    std::string smtpServer_;
    
    /** @brief The network port used for the SMTP handshake. */
    int smtpPort_;
    
    /** @brief The originating email address. */
    std::string senderEmail_;
    
    /** @brief Secure credentials for SMTP authentication. */
    std::string senderPassword_;
    
    /** @brief The target address for security notifications. */
    std::string recipientEmail_;
};