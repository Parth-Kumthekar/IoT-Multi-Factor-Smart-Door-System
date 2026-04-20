#include "EmailAlert.h"
#include <curl/curl.h>
#include <cstring>
#include <iostream>
#include <sstream>

namespace {
    struct UploadStatus {
        std::size_t bytesRead = 0;
        std::string payload;
    };

    std::size_t payloadSource(char* ptr, std::size_t size, std::size_t nmemb, void* userp) {
        UploadStatus* upload = static_cast<UploadStatus*>(userp);
        const std::size_t bufferSize = size * nmemb;
        if (upload->bytesRead >= upload->payload.size()) return 0;

        const std::size_t remaining = upload->payload.size() - upload->bytesRead;
        const std::size_t copySize = remaining < bufferSize ? remaining : bufferSize;
        std::memcpy(ptr, upload->payload.c_str() + upload->bytesRead, copySize);
        upload->bytesRead += copySize;
        return copySize;
    }

    std::string buildPayload(const std::string& from, const std::string& to,
                             const std::string& subject, const std::string& body) {
        std::ostringstream oss;
        oss << "To: <" << to << ">\r\nFrom: <" << from << ">\r\n"
            << "Subject: " << subject << "\r\n\r\n" << body << "\r\n";
        return oss.str();
    }
}

EmailAlert::EmailAlert(const std::string& smtpServer, int smtpPort,
                       const std::string& senderEmail, const std::string& senderPassword,
                       const std::string& recipientEmail)
    : smtpServer_(smtpServer), smtpPort_(smtpPort), senderEmail_(senderEmail),
      senderPassword_(senderPassword), recipientEmail_(recipientEmail) {}

bool EmailAlert::isConfigured() const {
    return !smtpServer_.empty() && !senderEmail_.empty() && !recipientEmail_.empty();
}

bool EmailAlert::sendIntrusionAlert(const std::string& subject, const std::string& body) {
    if (!isConfigured()) return false;
    CURL* curl = curl_easy_init();
    if (!curl) return false;

    const std::string smtpUrl = "smtp://" + smtpServer_ + ":" + std::to_string(smtpPort_);
    const std::string payload = buildPayload(senderEmail_, recipientEmail_, subject, body);
    UploadStatus uploadStatus{0, payload};

    struct curl_slist* recipients = nullptr;
    recipients = curl_slist_append(recipients, ("<" + recipientEmail_ + ">").c_str());

    curl_easy_setopt(curl, CURLOPT_URL, smtpUrl.c_str());
    curl_easy_setopt(curl, CURLOPT_USERNAME, senderEmail_.c_str());
    curl_easy_setopt(curl, CURLOPT_PASSWORD, senderPassword_.c_str());
    curl_easy_setopt(curl, CURLOPT_USE_SSL, (long)CURLUSESSL_ALL);
    curl_easy_setopt(curl, CURLOPT_MAIL_FROM, ("<" + senderEmail_ + ">").c_str());
    curl_easy_setopt(curl, CURLOPT_MAIL_RCPT, recipients);
    curl_easy_setopt(curl, CURLOPT_READFUNCTION, payloadSource);
    curl_easy_setopt(curl, CURLOPT_READDATA, &uploadStatus);
    curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);

    CURLcode res = curl_easy_perform(curl);
    curl_slist_free_all(recipients);
    curl_easy_cleanup(curl);
    return res == CURLE_OK;
}