#include "AsyncLogger.h"
#include <fstream>
#include <iostream>
#include <iomanip>
#include <chrono>
#include <sstream>
#include <curl/curl.h> 

/**
 * @brief Callback to suppress CURL output.
 */
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
    {
        std::lock_guard<std::mutex> lock(queueMtx_);
        running_ = false;
    }
    cv_.notify_all();
    if (logThread_.joinable()) {
        logThread_.join();
    }
}

/**
 * @brief FIX: Updated to support two arguments (Level and Message)
 * This matches the calls made in DoorAlarmSystem and FSM.
 */
void AsyncLogger::log(const std::string& level, const std::string& message) {
    std::string combined = "[" + level + "] " + message;
    {
        std::lock_guard<std::mutex> lock(queueMtx_);
        logQueue_.push(combined);
    }
    cv_.notify_one();
}

/**
 * @brief Consumer Loop with improved CURL handling
 */
void AsyncLogger::processLogs() {
    std::ofstream dbFile("security_audit.csv", std::ios::app);
    
    // Initialize CURL once at the start of the thread
    curl_global_init(CURL_GLOBAL_ALL);
    CURL* curl = curl_easy_init();

    if (dbFile.tellp() == 0) {
        dbFile << "Timestamp,Details\n";
    }

    while (true) {
        std::string rawMsg;

        {
            std::unique_lock<std::mutex> lock(queueMtx_);
            // Wait until queue is not empty or system is stopping
            cv_.wait(lock, [this] { return !running_ || !logQueue_.empty(); });
            
            // If stopping and queue is empty, exit loop
            if (!running_ && logQueue_.empty()) break;
            
            if (logQueue_.empty()) continue;

            rawMsg = logQueue_.front();
            logQueue_.pop();
        }

        // Timestamp generation
        auto now = std::chrono::system_clock::now();
        auto time_t_now = std::chrono::system_clock::to_time_t(now);
        std::ostringstream timeStream;
        // Use thread-safe localtime_r for Linux/Pi
        struct tm timeinfo;
        localtime_r(&time_t_now, &timeinfo);
        timeStream << std::put_time(&timeinfo, "%Y-%m-%d %H:%M:%S");
        std::string ts = timeStream.str();

        // 1. Console Output
        std::cout << "[" << ts << "] " << rawMsg << std::endl;

        // 2. CSV Persistence
        if (dbFile.is_open()) {
            dbFile << ts << ",\"" << rawMsg << "\"\n";
            dbFile.flush(); 
        }

        // 3. Cloud Logging (CURL)
        if (curl) {
            std::string payload = "{\"timestamp\":\"" + ts + "\", \"log\":\"" + rawMsg + "\"}";
            
            curl_easy_setopt(curl, CURLOPT_URL, "https://webhook.site/a794f19f-d0f5-4440-96fa-0703e27b76b8");
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payload.c_str());
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
            curl_easy_setopt(curl, CURLOPT_TIMEOUT, 2L); // Don't let a slow network hang the logger

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