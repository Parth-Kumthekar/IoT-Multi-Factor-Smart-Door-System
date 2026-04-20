#ifndef ASYNCLOGGER_H
#define ASYNCLOGGER_H

#include <string>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <atomic>
#include "EventQueue.h"

/**
 * @class AsyncLogger
 * @brief High-performance, non-blocking logger for real-time event recording.
 * * This class implements a producer-consumer pattern to offload file and 
 * console I/O to a dedicated background thread. This prevents disk latency 
 * from interfering with the real-time responsiveness of the door lock system.
 */
class AsyncLogger {
public:
    /**
     * @brief Constructor initializes the logger state.
     */
    AsyncLogger();

    /**
     * @brief Destructor ensures the logging thread is joined and resources are released.
     */
    ~AsyncLogger();

    /**
     * @brief Launches the background worker thread.
     */
    void start();

    /**
     * @brief Signals the worker thread to finish pending logs and shut down.
     */
    void stop();

    /**
     * @brief Submits a raw message to the logging queue.
     * @param message The string to be logged.
     */
    void log(const std::string& message);
    
    /**
     * @brief Formats and submits a categorized event to the logging queue.
     * @param category The event type (e.g., "NFC", "ALARM", "SYSTEM").
     * @param detail Specific information regarding the event.
     */
    void log(const std::string& category, const std::string& detail);

private:
    /**
     * @brief The background worker loop.
     * * Consumes messages from the queue and executes the physical write 
     * operations to the console and CSV file.
     */
    void processLogs();

    /** @brief Atomic flag to control the lifecycle of the worker thread. */
    std::atomic<bool> running_{false};

    /** @brief Dedicated thread for background I/O operations. */
    std::thread logThread_;
    
    /** @brief Internal storage for log messages awaiting processing. */
    std::queue<std::string> logQueue_;

    /** @brief Mutex to ensure thread-safe access to the log queue. */
    std::mutex queueMtx_;

    /** @brief Condition variable used to wake the worker thread only when data is available. */
    std::condition_variable cv_;

    /** * @brief Generates a high-resolution timestamp for log entries.
     * @return A string representing the current system time.
     */
    static std::string getTimestamp();
};

#endif