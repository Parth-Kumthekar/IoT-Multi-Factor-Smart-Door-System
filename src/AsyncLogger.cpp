#include "AsyncLogger.h"
#include <fstream>
#include <iostream>
#include <iomanip>
#include <chrono>
#include <sstream>
#include <curl/curl.h> 

/**
 * @brief Helper function to stop CURL from printing the server response to terminal.
 * @details This callback satisfies the CURLOPT_WRITEFUNCTION requirement by swallowing 
 * incoming data without writing it to stdout.
 * @return size_t The number of bytes "processed" (size * nmemb).
 */

size_t write_data(void *buffer, size_t size, size_t nmemb, void *userp) {
    (void)buffer; 
    (void)userp;
    return size * nmemb;
}

/**
 * @brief Construct a new Async Logger.
 * @details Initializes the running state to false. The worker thread is not started until start() is called.
 */
AsyncLogger::AsyncLogger() : running_(false) {}

/**
 * @brief Destroy the Async Logger.
 * @details Ensures the worker thread is signaled to stop and joined before the object is destroyed.
 */
AsyncLogger::~AsyncLogger() {
    stop();
}

/**
 * @brief Spawns the background logging thread.
 * @details If the logger is already running, this function returns immediately to prevent multiple threads.
 */
void AsyncLogger::start() {
    if (running_) return;
    running_ = true;
    logThread_ = std::thread(&AsyncLogger::processLogs, this);
}

/**
 * @brief Signals the logging thread to exit.
 * @details Sets the running flag to false and notifies the condition variable to wake the worker thread.
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
 * @brief Enqueues a message for asynchronous processing.
 * @details Thread-safely pushes the message into the queue and notifies the worker thread.
 * @param message The string content to be logged.
 */
void AsyncLogger::log(const std::string& message) {
    {
        std::lock_guard<std::mutex> lock(queueMtx_);
        logQueue_.push(message);
    }
    cv_.notify_one();
}

/**
 * @brief The worker thread loop that handles I/O operations.
 * @details This method performs three primary actions for every log entry:
 * 1. Formats a timestamp.
 * 2. Writes the log to the standard console output.
 * 3. Appends the log to a local CSV file ("security_audit.csv").
 * 4. Dispatches an HTTPS POST request with a JSON payload to a remote webhook.
 */
void AsyncLogger::processLogs() {
    std::ofstream dbFile("security_audit.csv", std::ios::app);
    
    curl_global_init(CURL_GLOBAL_ALL);
    CURL* curl = curl_easy_init();

    // Initialize CSV header if the file is new
    if (dbFile.tellp() == 0) {
        dbFile << "Timestamp,Category,Details\n";
    }

    while (running_ || !logQueue_.empty()) {
        std::string rawMsg;

        {
            std::unique_lock<std::mutex> lock(queueMtx_);
            // Wait for new logs or a shutdown signal
            cv_.wait(lock, [this] { return !running_ || !logQueue_.empty(); });
            
            if (logQueue_.empty()) continue;
            
            rawMsg = logQueue_.front();
            logQueue_.pop();
        }

        // Generate Timestamp
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
            curl_easy_setopt(curl, CURLOPT_URL, "http://192.168.1.22/api/logs");
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

    // Cleanup resources
    if (curl) curl_easy_cleanup(curl);
    if (dbFile.is_open()) dbFile.close();
    curl_global_cleanup();
}
