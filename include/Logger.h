#pragma once
#include <thread>
#include <fstream>
#include <string>
#include "ThreadSafeQueue.h"
#include "AccessEvent.h"

class Logger {
public:
    explicit Logger(const std::string& csvPath = "database/access_log.csv");
    ~Logger();                  

    Logger(const Logger&)            = delete;
    Logger& operator=(const Logger&) = delete;

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