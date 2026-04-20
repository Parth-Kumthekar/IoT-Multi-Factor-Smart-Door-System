#include "AsyncLogger.h"
#include <fstream>
#include <iostream>
#include <iomanip>
#include <chrono>
#include <sstream>
#include <curl/curl.h> 

/**
 * @brief Callback to suppress CURL output.
 * * Required to maintain a clean terminal interface by diverting server 
 * responses to a null sink.
 */
size_t write_data(void *buffer, size_t size, size_t nmemb, void *userp) {
    return size * nmemb;
}

AsyncLogger::AsyncLogger() : running_(false) {}

AsyncLogger::~AsyncLogger() {
    stop();
}

/**
 * @brief Initializes the background worker thread.
 */
void AsyncLogger::start() {
    if (running_) return;
    running_ = true;
    logThread_ = std::thread(&AsyncLogger::processLogs, this);
}

/**
 * @brief Signals the thread to exit and joins it.
 * * Uses a condition variable notification to wake the thread immediately 
 * from its wait state for a clean shutdown.
 */
void AsyncLogger::stop() {
    if (!running_) return;
    running_ = false;
    cv_.notify_all();
    if (logThread_.joinable()) {
        logThread_.join();
    }
}

/**
 * @brief Thread-safe method to submit logs.
 * * Implements a non-blocking producer: it merely pushes data to a queue 
 * and notifies the worker, ensuring zero latency for the caller.
 */
void AsyncLogger::log(const std::string& message) {
    {
        std::lock_guard<std::mutex> lock(queueMtx_);
        logQueue_.push(message);
    }
    cv_.notify_one();
}

/**
 * @brief Main Consumer Loop.
 * * Handles the three pillars of system audit: Console output, 
 * Local CSV persistence, and Remote Cloud logging via HTTPS.
 */
void AsyncLogger::processLogs() {
    std::ofstream dbFile("security_audit.csv", std::ios::app);
    
    // Initialize network resources
    curl_global_init(CURL_GLOBAL_ALL);
    CURL* curl = curl_easy_init();

    // CSV Header initialization
    if (dbFile.tellp() == 0) {
        dbFile << "Timestamp,Category,Details\n";
    }

    while (running_ || !logQueue_.empty()) {
        std::string rawMsg;

        {
            // Thread suspension: wait for work or shutdown signal
            std::unique_lock<std::mutex> lock(queueMtx_);
            cv_.wait(lock, [this] { return !running_ || !logQueue_.empty(); });
            
            if (logQueue_.empty() && !running_) break;
            if (logQueue_.empty()) continue;

            rawMsg = logQueue_.front();
            logQueue_.pop();
        }

        // Timestamp generation (Thread-safe formatting)
        auto now = std::chrono::system_clock::now();
        auto time_t_now = std::chrono::system_clock::to_time_t(now);
        std::ostringstream timeStream;
        timeStream << std::put_time(std::localtime(&time_t_now), "%Y-%m-%d %H:%M:%S");
        std::string ts = timeStream.str();

        // Prepare JSON payload for Cloud API
        std::string payload = "{\"timestamp\":\"" + ts + "\", \"log\":\"" + rawMsg + "\"}";

        // 1. Terminal Output: Real-time monitoring for the operator
        std::cout << "[" << ts << "] " << rawMsg << std::endl;

        // 2. Local CSV Persistence: High-reliability local audit trail
        if (dbFile.is_open()) {
            dbFile << ts << ",\"" << rawMsg << "\"\n";
            dbFile.flush(); // Ensure data is written even if power fails
        }

        // 3. Remote HTTPS Integration (Cloud Logging)
        if (curl) {
            curl_easy_setopt(curl, CURLOPT_URL, "https://webhook.site/a794f19f-d0f5-4440-96fa-0703e27b76b8");
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payload.c_str());
            
            // Redirect write to helper to avoid terminal clutter
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);

            struct curl_slist *headers = NULL;
            headers = curl_slist_append(headers, "Content-Type: application/json");
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
            
            // Note: SSL verification disabled for lab testing compatibility
            curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);

            curl_easy_perform(curl);
            curl_slist_free_all(headers);
        }
    }

    // Clean up resources in reverse order of initialization
    if (curl) curl_easy_cleanup(curl);
    if (dbFile.is_open()) dbFile.close();
    curl_global_cleanup();
}