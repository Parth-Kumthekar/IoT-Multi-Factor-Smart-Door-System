#pragma once
#include <thread>
#include <fstream>
#include <string>
#include "ThreadSafeQueue.h"
#include "AccessEvent.h"

class AsyncLogger {
public:
    explicit AsyncLogger(const std::string& csvPath = "database/access_log.csv");
    ~AsyncLogger();                  

    AsyncLogger(const AsyncLogger&)            = delete;
    AsyncLogger& operator=(const AsyncLogger&) = delete;

    // Thread-safe
    void log(AccessEvent ev);

    const std::vector<AccessEvent>& entries() const;

private:
    void workerLoop();               

    std::string                  csvPath_;
    ThreadSafeQueue<AccessEvent> queue_;
    std::vector<AccessEvent>     entries_;
    mutable std::mutex           entriesMutex_;
    std::thread                  worker_;
    std::ofstream                csv_;
};