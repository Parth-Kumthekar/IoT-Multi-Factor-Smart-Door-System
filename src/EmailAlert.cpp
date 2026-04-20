#include "EmailAlert.h"
#include <curl/curl.h>
#include <cstring>
#include <iostream>
#include <sstream>

namespace {
    /**
     * @struct UploadStatus
     * @brief Context for the CURL read callback.
     * * Keeps track of how much of the email payload has been transmitted.
     */
    struct UploadStatus {
        std::size_t bytesRead = 0;
        std::string payload;
    };

    /**
     * @brief CURL Callback function to provide the email content.
     * * CURL calls this repeatedly until it has read the entire payload string.
     */
    std::size_t payloadSource(char* ptr, std::size_t size, std::size_t nmemb, void* userp) {
        UploadStatus* upload = static_cast<UploadStatus*>(userp);
        const std::size_t bufferSize = size * nmemb;
        
        if (upload->bytesRead >= upload->payload.size()) return 0;

        const std::size_t remaining = upload->payload.size() - upload->bytesRead;
        const std::size_t copySize = (remaining < bufferSize) ? remaining : bufferSize;
        
        std::memcpy(ptr, upload->payload.c_str() + upload->bytesRead, copySize);
        upload->bytesRead += copySize;
        
        return copySize;
    }

    /**
     * @brief Formats raw text into a valid SMTP message with headers.
     * * Crucial for avoiding spam filters and ensuring the 'Subject' line displays.
     */
    std::string buildPayload(const std::string& from, const std::string& to,
                             const std::string& subject, const std::string& body) {
        std::ostringstream oss;
        // Standard SMTP structure: Headers -> CRLF -> Body
        oss << "To: " << to << "\r\n"
            << "From: " << from << "\r\n"
            << "Subject: " << subject << "\r\n"
            << "MIME-Version: 1.0\r\n"
            << "Content-Type: text/plain; charset=\"utf-8\"\r\n"
            << "\r\n" 
            << body << "\r\n";
        return oss.str();
    }
}

EmailAlert::EmailAlert() 
    : smtpServer_("smtp.gmail.com"), 
      smtpPort_(587), 
      senderEmail_("your-team-email@gmail.com"),
      senderPassword_("your-app-password"), 
      recipientEmail_("your-personal-email@gmail.com") 
{}

EmailAlert::EmailAlert(const std::string& smtpServer, int smtpPort,
                       const std::string& senderEmail, const std::string& senderPassword,
                       const std::string& recipientEmail)
    : smtpServer_(smtpServer), smtpPort_(smtpPort), senderEmail_(senderEmail),
      senderPassword_(senderPassword), recipientEmail_(recipientEmail) {}

/**
 * @brief Dispatches the email via SMTP.
 * * This is a network-blocking call. It should always be executed within 
 * the background logic (e.g., AsyncLogger thread) to prevent FSM lag.
 */
bool EmailAlert::send(const std::string& subject, const std::string& body) {
    CURL* curl = curl_easy_init();
    if (!curl) return false;

    const std::string smtpUrl = "smtp://" + smtpServer_ + ":" + std::to_string(smtpPort_);
    const std::string payload = buildPayload(senderEmail_, recipientEmail_, subject, body);
    UploadStatus uploadStatus{0, payload};

    struct curl_slist* recipients = nullptr;
    recipients = curl_slist_append(recipients, recipientEmail_.c_str());

    // Connection setup
    curl_easy_setopt(curl, CURLOPT_URL, smtpUrl.c_str());
    curl_easy_setopt(curl, CURLOPT_USERNAME, senderEmail_.c_str());
    curl_easy_setopt(curl, CURLOPT_PASSWORD, senderPassword_.c_str());
    
    // Security: Mandatory STARTTLS for Gmail/Outlook
    curl_easy_setopt(curl, CURLOPT_USE_SSL, (long)CURLUSESSL_ALL);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L); // Lab environment bypass
    
    // SMTP Logic
    curl_easy_setopt(curl, CURLOPT_MAIL_FROM, senderEmail_.c_str());
    curl_easy_setopt(curl, CURLOPT_MAIL_RCPT, recipients);
    curl_easy_setopt(curl, CURLOPT_READFUNCTION, payloadSource);
    curl_easy_setopt(curl, CURLOPT_READDATA, &uploadStatus);
    curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
    
    // Failsafe: Don't hang the thread forever if Wi-Fi is down
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L); 

    CURLcode res = curl_easy_perform(curl);

    if (res != CURLE_OK) {
        // Log locally to stderr if the cloud notification fails
        std::cerr << "[CLOUD ERROR] Email failed: " << curl_easy_strerror(res) << std::endl;
    }

    curl_slist_free_all(recipients);
    curl_easy_cleanup(curl);
    return res == CURLE_OK;
}