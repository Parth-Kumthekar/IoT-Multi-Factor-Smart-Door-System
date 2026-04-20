#ifndef ASYNCLOGGER_H
#define ASYNCLOGGER_H

#include <string>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <atomic>
#include "EventQueue.h" // Assuming you use your SafeQueue/EventQueue template

class AsyncLogger {
public:
    AsyncLogger();
    ~AsyncLogger();

    // Lifecycle
    void start();
    void stop();

    /**
     * Main log function used by FSM and AlarmManager.
     * Use format "CATEGORY: Message" for best CSV results.
     */
    void log(const std::string& message);
    
    // Overload for convenience
    void log(const std::string& category, const std::string& detail);

private:
    /**
     * The worker thread function that writes to console and CSV.
     */
    void processLogs();

    std::atomic<bool> running_{false};
    std::thread logThread_;
    
    // Use a thread-safe queue (either your EventQueue or a local one)
    std::queue<std::string> logQueue_;
    std::mutex queueMtx_;
    std::condition_variable cv_;

    // Helper for time formatting
    static std::string getTimestamp();
};

#endif