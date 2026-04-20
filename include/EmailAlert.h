#pragma once
#include <string>

class EmailAlert {
public:
    EmailAlert(const std::string& smtpServer,
               int smtpPort,
               const std::string& senderEmail,
               const std::string& senderPassword,
               const std::string& recipientEmail);

    bool isConfigured() const;
    bool sendIntrusionAlert(const std::string& subject, const std::string& body);

private:
    std::string smtpServer_;
    int smtpPort_;
    std::string senderEmail_;
    std::string senderPassword_;
    std::string recipientEmail_;
};