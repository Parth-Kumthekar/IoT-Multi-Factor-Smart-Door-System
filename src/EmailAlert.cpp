#include "EmailAlert.h"

#include <curl/curl.h>

#include <cstring>
#include <iostream>
#include <sstream>
#include <string>

namespace {
    struct UploadStatus {
        std::size_t bytesRead = 0;
        std::string payload;
    };

    std::size_t payloadSource(char* ptr, std::size_t size, std::size_t nmemb, void* userp) {
        UploadStatus* upload = static_cast<UploadStatus*>(userp);
        const std::size_t bufferSize = size * nmemb;

        if (upload->bytesRead >= upload->payload.size()) {
            return 0;
        }

        const std::size_t remaining = upload->payload.size() - upload->bytesRead;
        const std::size_t copySize = remaining < bufferSize ? remaining : bufferSize;

        std::memcpy(ptr, upload->payload.c_str() + upload->bytesRead, copySize);
        upload->bytesRead += copySize;
        return copySize;
    }

    std::string buildPayload(const std::string& from,
                             const std::string& to,
                             const std::string& subject,
                             const std::string& body) {
        std::ostringstream oss;
        oss << "To: <" << to << ">\r\n";
        oss << "From: <" << from << ">\r\n";
        oss << "Subject: " << subject << "\r\n";
        oss << "\r\n";
        oss << body << "\r\n";
        return oss.str();
    }
}

EmailAlert::EmailAlert(const std::string& smtpServer,
                       int smtpPort,
                       const std::string& senderEmail,
                       const std::string& senderPassword,
                       const std::string& recipientEmail)
    : smtpServer_(smtpServer),
      smtpPort_(smtpPort),
      senderEmail_(senderEmail),
      senderPassword_(senderPassword),
      recipientEmail_(recipientEmail) {
}

bool EmailAlert::isConfigured() const {
    return !smtpServer_.empty()
        && smtpPort_ > 0
        && !senderEmail_.empty()
        && !senderPassword_.empty()
        && !recipientEmail_.empty();
}

bool EmailAlert::sendIntrusionAlert(const std::string& subject,
                                    const std::string& body) {
    if (!isConfigured()) {
        std::cerr << "EmailAlert is not configured." << std::endl;
        return false;
    }

    CURL* curl = curl_easy_init();
    if (!curl) {
        std::cerr << "Failed to initialize CURL." << std::endl;
        return false;
    }

    const std::string smtpUrl = "smtp://" + smtpServer_ + ":" + std::to_string(smtpPort_);
    const std::string payload = buildPayload(senderEmail_, recipientEmail_, subject, body);

    UploadStatus uploadStatus;
    uploadStatus.payload = payload;

    struct curl_slist* recipients = nullptr;
    recipients = curl_slist_append(recipients, ("<" + recipientEmail_ + ">").c_str());

    curl_easy_setopt(curl, CURLOPT_URL, smtpUrl.c_str());
    curl_easy_setopt(curl, CURLOPT_USERNAME, senderEmail_.c_str());
    curl_easy_setopt(curl, CURLOPT_PASSWORD, senderPassword_.c_str());

    // Use STARTTLS for SMTP
    curl_easy_setopt(curl, CURLOPT_USE_SSL, static_cast<long>(CURLUSESSL_ALL));

    curl_easy_setopt(curl, CURLOPT_MAIL_FROM, ("<" + senderEmail_ + ">").c_str());
    curl_easy_setopt(curl, CURLOPT_MAIL_RCPT, recipients);

    curl_easy_setopt(curl, CURLOPT_READFUNCTION, payloadSource);
    curl_easy_setopt(curl, CURLOPT_READDATA, &uploadStatus);
    curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);

    CURLcode result = curl_easy_perform(curl);

    bool success = (result == CURLE_OK);
    if (!success) {
        std::cerr << "Email sending failed: " << curl_easy_strerror(result) << std::endl;
    }

    curl_slist_free_all(recipients);
    curl_easy_cleanup(curl);

    return success;
}