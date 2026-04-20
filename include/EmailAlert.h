#pragma once
#include <string>

class EmailAlert {
public:
    /**
     * Default Constructor
     * Uses the hardcoded Team 22 credentials defined in EmailAlert.cpp
     */
    EmailAlert();

    /**
     * Parameterized Constructor 
     * Allows overriding credentials at runtime if needed
     */
    EmailAlert(const std::string& smtpServer,
               int smtpPort,
               const std::string& senderEmail,
               const std::string& senderPassword,
               const std::string& recipientEmail);

    /**
     * Main sending function called by AlarmManager
     */
    bool send(const std::string& subject, const std::string& body);

    bool isConfigured() const;

private:
    std::string smtpServer_;
    int smtpPort_;
    std::string senderEmail_;
    std::string senderPassword_;
    std::string recipientEmail_;
};