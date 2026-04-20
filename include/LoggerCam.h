#pragma once
#include <thread>
#include <fstream>
#include <string>
#include "ThreadSafeQueue.h"
#include "AccessEvent.h"

class AsyncLoggerCam {
public:
    explicit AsyncLoggerCam(const std::string& csvPath = "database/access_log.csv");
    ~AsyncLoggerCam();                  

    AsyncLoggerCam(const AsyncLoggerCam&)            = delete;
    AsyncLoggerCam& operator=(const AsyncLoggerCam&) = delete;

   
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
