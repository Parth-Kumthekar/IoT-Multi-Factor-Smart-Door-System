#include "Logger.h"
#include <iomanip>
#include <sstream>
#include <iostream>
#include <filesystem>

static std::string timeStr(std::chrono::system_clock::time_point tp) {
    auto t = std::chrono::system_clock::to_time_t(tp);
    std::ostringstream ss;
    ss << std::put_time(std::localtime(&t), "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

AsyncLogger::AsyncLogger(const std::string& csvPath) : csvPath_(csvPath) {
    std::filesystem::create_directories("database");
    csv_.open(csvPath_, std::ios::app);
    if (csv_.tellp() == 0)
        csv_ << "Time,Person,Method,Result,Confidence,Note\n";

    // THREAD
    worker_ = std::thread(&AsyncLogger::workerLoop, this);
}

AsyncLogger::~AsyncLogger() {
    queue_.shutdown();               
    if (worker_.joinable())
        worker_.join();              
    csv_.flush();
}

void AsyncLogger::log(AccessEvent ev) {
    queue_.push(std::move(ev));      
}

void AsyncLogger::workerLoop() {
    while (true) {
        auto ev = queue_.pop();      
        if (!ev) break;              

        
        csv_ << '"' << timeStr(ev->timestamp) << "\","
             << '"' << ev->identity           << "\","
             << '"' << AccessEvent::methodStr(ev->method) << "\","
             << '"' << AccessEvent::resultStr(ev->result) << "\","
             << std::fixed << std::setprecision(3) << ev->confidence << ','
             << '"' << ev->note               << "\"\n";
        csv_.flush();

        // Print to console
        std::cout << "[LOG] " << timeStr(ev->timestamp)
                  << " | " << AccessEvent::methodStr(ev->method)
                  << " | " << ev->identity
                  << " | " << AccessEvent::resultStr(ev->result);
        if (!ev->note.empty()) std::cout << " | " << ev->note;
        std::cout << '\n';

        // Store in memory for GUI/API
        {
            std::lock_guard<std::mutex> lk(entriesMutex_);
            entries_.push_back(*ev);
        }
    }
}

const std::vector<AccessEvent>& AsyncLogger::entries() const {
    return entries_; 
}