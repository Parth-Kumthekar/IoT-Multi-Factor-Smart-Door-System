#ifndef ASYNCLOGGER_H
#define ASYNCLOGGER_H

#include <string>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <atomic>
#include "EventQueue.h" // Assuming you use your SafeQueue/EventQueue template

/**
 * @class AsyncLogger
 * @brief Provides non-blocking logging capabilities using a dedicated worker thread.
 * * This class offloads I/O operations (writing to console/CSV) to a background thread
 * to prevent logging latency from affecting the main execution flow of the FSM or AlarmManager.
 */
class AsyncLogger {
public:
    /**
     * @brief Construct a new Async Logger object.
     */
    AsyncLogger();

    /**
     * @brief Destroy the Async Logger object.
     * @note Automatically calls stop() to ensure the worker thread is joined.
     */
    ~AsyncLogger();

    /**
     * @brief Starts the background logging thread.
     */
    void start();

    /**
     * @brief Signals the worker thread to stop and joins it.
     */
    void stop();

    /**
     * @brief Main log function used by FSM and AlarmManager.
     * @details Pushes a message into the internal queue for asynchronous processing.
     * @param message The full string to log. Use format "CATEGORY: Message" for best CSV results.
     */
    void log(const std::string& message);
    
    /**
     * @brief Overload for convenience to separate category and details.
     * @param category The high-level category (e.g., "SYSTEM", "ALARM").
     * @param detail Specific information regarding the event.
     */
    void log(const std::string& category, const std::string& detail);

private:
    /**
     * @brief The worker thread function that writes to console and CSV.
     * @details This function runs in a loop until running_ is false and the queue is empty.
     */
    void processLogs();

    /// Atomic flag controlling the lifecycle of the worker thread.
    std::atomic<bool> running_{false};

    /// The handle for the background processing thread.
    std::thread logThread_;
    
    /// Internal buffer for log messages awaiting disk/console I/O.
    std::queue<std::string> logQueue_;

    /// Mutex to protect access to the logQueue_.
    std::mutex queueMtx_;

    /// Condition variable used to wake the worker thread when new logs arrive.
    std::condition_variable cv_;

    /**
     * @brief Helper for time formatting.
     * @return std::string A formatted ISO-8601 or similar timestamp.
     */
    static std::string getTimestamp();
};

#endif
